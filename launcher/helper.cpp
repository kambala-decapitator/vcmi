/*
 * helper.cpp, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#include "StdInc.h"
#include "helper.h"

#include "mainwindow_moc.h"
#include "settingsView/csettingsview_moc.h"
#include "modManager/cmodlistview_moc.h"

#include "../lib/CConfigHandler.h"

#include <QObject>
#include <QScroller>
#include <QFileDialog>

#ifdef VCMI_ANDROID
#include "android/androidutils.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAndroidActivityResultReceiver>
#endif
#elif defined(VCMI_IOS)
#include "ios/revealdirectoryinfiles.h"
#include "ios/selectdirectory.h"
#include "iOS_utils.h"
#endif

#ifdef VCMI_MOBILE
static QScrollerProperties generateScrollerProperties()
{
	QScrollerProperties result;

	result.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.25);
	result.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.25);
	result.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);

	return result;
}
#endif

#ifdef VCMI_ANDROID
static QString safeEncode(QString uri)
{
	// %-encode unencoded parts of string.
	// This is needed because Qt returns a mixed content url with %-encoded and unencoded parts. On Android >= 13 this causes problems reading these files, when using spaces and unicode characters in folder or filename.
	// Only these should be encoded (other typically %-encoded chars should not be encoded because this leads to errors).
	// Related, but seems not completly fixed (at least in our setup): https://bugreports.qt.io/browse/QTBUG-114435
	if (!uri.startsWith("content://", Qt::CaseInsensitive))
		return uri;
	return QString::fromUtf8(QUrl::toPercentEncoding(uri, "!#$&'()*+,/:;=?@[]<>{}\"`^~%"));
}
#endif

namespace Helper
{
void loadSettings()
{
	settings.init("config/settings.json", "vcmi:settings");
	persistentStorage.init("config/persistentStorage.json", "");
}

void reLoadSettings()
{
	loadSettings();
	for(const auto widget : qApp->allWidgets())
		if(auto settingsView = qobject_cast<CSettingsView *>(widget))
		{
			settingsView->loadSettings();
			break;
		}
	getMainWindow()->updateTranslation();
	getMainWindow()->getModView()->reload();
}

void enableScrollBySwiping(QObject * scrollTarget)
{
#ifdef VCMI_MOBILE
	QScroller::grabGesture(scrollTarget, QScroller::LeftMouseButtonGesture);
	QScroller * scroller = QScroller::scroller(scrollTarget);
	scroller->setScrollerProperties(generateScrollerProperties());
#endif
}

QString getRealPath(QString path)
{
#ifdef VCMI_ANDROID
	if(path.contains("content://", Qt::CaseInsensitive))
	{
		auto str = QJNI_OBJECT_CLASS::fromString(safeEncode(path));
		return QJNI_OBJECT_CLASS::callStaticObjectMethod("eu/vcmi/vcmi/util/FileUtil", "getFilenameFromUri", "(Ljava/lang/String;Landroid/content/Context;)Ljava/lang/String;", str.object<jstring>(), QT_ACTIVITY.object()).toString();
	}
	return path;
#else
	return path;
#endif
}

bool performNativeCopy(QString src, QString dst)
{
#ifdef VCMI_ANDROID
	const bool srcIsContent = src.startsWith("content://", Qt::CaseInsensitive);
	const bool dstIsContent = dst.startsWith("content://", Qt::CaseInsensitive);

	if(srcIsContent && !dstIsContent)
	{
		const QJNI_OBJECT_CLASS jSrc = QJNI_OBJECT_CLASS::fromString(safeEncode(src));
		const QJNI_OBJECT_CLASS jDst = QJNI_OBJECT_CLASS::fromString(dst);
		QJNI_OBJECT_CLASS::callStaticMethod<void>("eu/vcmi/vcmi/util/FileUtil", "copyFileFromUri", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/Context;)V",	jSrc.object<jstring>(),	jDst.object<jstring>(),	QT_ACTIVITY.object());
		// Real filesystem path -> QFileInfo works
		return QFileInfo(dst).exists() && QFileInfo(dst).size() > 0;
	}

	// filesystem path -> content://  (NEW Java method that writes via ContentResolver)
	if(!srcIsContent && dstIsContent)
	{
		const QJNI_OBJECT_CLASS jSrc = QJNI_OBJECT_CLASS::fromString(src);
		const QJNI_OBJECT_CLASS jDst = QJNI_OBJECT_CLASS::fromString(safeEncode(dst));
		const jboolean ok = QJNI_OBJECT_CLASS::callStaticMethod<jboolean>("eu/vcmi/vcmi/util/FileUtil", "copyFilePathToUri", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/Context;)Z", jSrc.object<jstring>(), jDst.object<jstring>(), QT_ACTIVITY.object());
		return ok;
	}
#endif

	// Pure filesystem -> use Qt copy
	QFile::remove(dst);
	return QFile::copy(src, dst);
}


QString createFile(QString target, QString fileName, QString mime)
{
#ifdef VCMI_ANDROID
	if(target.startsWith("content://", Qt::CaseInsensitive))
	{
		const QJNI_OBJECT_CLASS jTree = QJNI_OBJECT_CLASS::fromString(target);
		const QJNI_OBJECT_CLASS jName = QJNI_OBJECT_CLASS::fromString(fileName);
		const QString targetMime = mime.isEmpty() ? QStringLiteral("application/octet-stream") : mime;
		const QJNI_OBJECT_CLASS jMime = QJNI_OBJECT_CLASS::fromString(targetMime);
		const QJNI_OBJECT_CLASS jDst = QJNI_OBJECT_CLASS::callStaticObjectMethod(
			"eu/vcmi/vcmi/util/FileUtil",
			"createFileInTree",
			"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/content/Context;)Ljava/lang/String;",
			jTree.object<jstring>(),
			jName.object<jstring>(),
			jMime.object<jstring>(),
			QT_ACTIVITY.object());

		return jDst.isValid() ? jDst.toString() : QString();
	}
#endif

	return QDir(target).filePath(fileName);
}

void revealDirectoryInFileBrowser(QString path)
{
	const auto dirUrl = QUrl::fromLocalFile(QFileInfo{path}.absoluteFilePath());
#ifdef VCMI_IOS
	iOS_utils::revealDirectoryInFiles(dirUrl);
#else
	QDesktopServices::openUrl(dirUrl);
#endif
}

MainWindow * getMainWindow()
{
	foreach(QWidget *w, qApp->allWidgets())
		if(auto mainWin = qobject_cast<MainWindow*>(w))
			return mainWin;
	return nullptr;
}

void keepScreenOn(bool isEnabled)
{
#if defined(VCMI_ANDROID)
	RUN_ON_ANDROID_MAIN_THREAD([isEnabled]
	{
		JNI_CALL_METHOD(QT_ACTIVITY, void, "keepScreenOn", "(Z)V", isEnabled);
	});
#elif defined(VCMI_IOS)
	iOS_utils::keepScreenOn(isEnabled);
#endif
}

void allowGamepadStart(bool isEnabled)
{
#if defined(VCMI_ANDROID)
	RUN_ON_ANDROID_MAIN_THREAD([isEnabled]
	{
		JNI_CALL_METHOD(QT_ACTIVITY, void, "setGamepadStartEnabled", "(Z)V", isEnabled);
	});
#endif
}

bool canUseFolderPicker()
{
#if defined(VCMI_ANDROID)
	// Folder picker is available on API >= 21.
	// Android/Google TV usually lacks DocumentsUI — hide this option.

	QJNI_OBJECT_CLASS context = QT_ACTIVITY;
	QJNI_OBJECT_CLASS uiModeMgr = context.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", QJNI_OBJECT_CLASS::fromString("uimode").object<jstring>());

	if(uiModeMgr.isValid())
	{
		jint mode = JNI_CALL_METHOD(uiModeMgr, jint, "getCurrentModeType", "()I");
		jint TV = QJNI_OBJECT_CLASS::getStaticField<jint>("android/content/res/Configuration", "UI_MODE_TYPE_TELEVISION");
		if(mode == TV)
			return false;
	}

	return true;
#elif defined(VCMI_IOS)
	// selecting directory through UIDocumentPickerViewController is available only since iOS 13
	return iOS_utils::isOsVersionAtLeast(13);
#else
	return true;
#endif
}

#ifdef VCMI_ANDROID
// Request code for Android folder picker (ACTION_OPEN_DOCUMENT_TREE).
// Value is arbitrary, used only to match activity result callback.
static constexpr int  kFolderPickerReqCode = 4242;

static jint intentFlags()
{
	const jint fRead	= QJNI_OBJECT_CLASS::getStaticField<jint>("android/content/Intent", "FLAG_GRANT_READ_URI_PERMISSION");
	const jint fWrite   = QJNI_OBJECT_CLASS::getStaticField<jint>("android/content/Intent", "FLAG_GRANT_WRITE_URI_PERMISSION");
	const jint fPersist = QJNI_OBJECT_CLASS::getStaticField<jint>("android/content/Intent", "FLAG_GRANT_PERSISTABLE_URI_PERMISSION");
	const jint fPrefix  = QJNI_OBJECT_CLASS::getStaticField<jint>("android/content/Intent", "FLAG_GRANT_PREFIX_URI_PERMISSION");

	return fRead | fWrite | fPersist | fPrefix;
}

class FolderPickReceiver final : public QAndroidActivityResultReceiver
{
public:
	std::function<void(QString)> onDone;

	// One-shot result handler for ACTION_OPEN_DOCUMENT_TREE
	void handleActivityResult(int req, int res, const QJNI_OBJECT_CLASS &data) override
	{
		auto cb = std::exchange(onDone, {}); // guarantee single-use
		if(!cb)
			return;

		if(req != kFolderPickerReqCode || res != -1 /*RESULT_OK*/ || !data.isValid())
		{
			QMetaObject::invokeMethod(qApp, [cb]{ if (cb) cb({}); }, Qt::QueuedConnection);
			return;
		}

		// Always return content:// tree URI
		const auto uri = data.callObjectMethod("getData", "()Landroid/net/Uri;");
		const auto us = uri.callObjectMethod("toString", "()Ljava/lang/String;");
		const QString pickedTree = us.toString();

		// Persist read+write permission
		// TODO: not working
		// java.lang.SecurityException: Permission Denial: reading com.android.externalstorage.ExternalStorageProvider uri
		const QJNI_OBJECT_CLASS ctx = QT_ACTIVITY;
		const auto cr = ctx.callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");
		cr.callMethod<void>("takePersistableUriPermission", "(Landroid/net/Uri;I)V", uri.object(), jint(1 | 2));
		// JNI_CALL_METHOD(cr, void, "takePersistableUriPermission", "(Landroid/net/Uri;I)V", uri.object<jobject>(), jint(1 | 2));

		// Bounce back to Qt thread
		QMetaObject::invokeMethod(qApp, [cb, pickedTree]{ if (cb) cb(pickedTree); }, Qt::QueuedConnection);
	}
};

static FolderPickReceiver g_receiver;

#endif

void nativeFolderPicker(QWidget *parent, std::function<void(QString)>&& cb)
{
	if(!cb)
		return;

#if defined(VCMI_ANDROID)
	Q_UNUSED(parent);
	g_receiver.onDone = std::move(cb);

	QJNI_OBJECT_CLASS intent("android/content/Intent","()V");
	intent.callObjectMethod("setAction", "(Ljava/lang/String;)Landroid/content/Intent;", QJNI_OBJECT_CLASS::fromString("android.intent.action.OPEN_DOCUMENT_TREE").object<jstring>());
	intent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", intentFlags());

	START_ACTIVITY(intent, kFolderPickerReqCode, &g_receiver);

#elif defined(VCMI_IOS)
	SelectDirectory iosDirectorySelector;
	const QString dir = iosDirectorySelector.getExistingDirectory();
	cb(dir);

#else
	const QString dir = QFileDialog::getExistingDirectory(parent, {}, {}, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	cb(dir);
#endif
}

static inline QString classifyTargetByExt(const QString &baseName)
{
	// Case-insensitive suffix checks without making a lowercase copy
	auto ends = [&](const char *s){ return baseName.endsWith(QLatin1String(s), Qt::CaseInsensitive); };

	if(ends(".lod") || ends(".snd") || ends(".vid") || ends(".pak"))
		return QStringLiteral("Data");

	if(ends(".h3m") || ends(".tut"))
		return QStringLiteral("Maps");

	if(ends(".mp3"))
		return QStringLiteral("Mp3");

	return {};
}

static void addIfExists(QVector<QDir> &scan, const QDir &base, const char *child)
{
	QDir dir(base.filePath(QLatin1String(child)));
	if(dir.exists())
		scan << dir;
}

QStringList findFilesForCopy(const QString &path)
{
#ifdef VCMI_ANDROID
	if(path.startsWith(QLatin1String("content://"), Qt::CaseInsensitive))
	{
		const QJNI_OBJECT_CLASS jUri = QJNI_OBJECT_CLASS::fromString(safeEncode(path));
		const QJNI_OBJECT_CLASS jArr = QJNI_OBJECT_CLASS::callStaticObjectMethod("eu/vcmi/vcmi/util/FileUtil", "findFilesForCopy", "(Ljava/lang/String;Landroid/content/Context;)[Ljava/lang/String;", jUri.object<jstring>(), QT_ACTIVITY.object());

		QStringList out;
		if(!jArr.isValid())
			return out;

		const jobjectArray arr = static_cast<jobjectArray>(jArr.object<jobject>());
		android::withJniEnv([&](JNIEnv * jniEnv)
		{
			const jsize n = jniEnv->GetArrayLength(arr);
			out.reserve(n);

			for(jsize i = 0; i < n; ++i)
			{
				QJNI_OBJECT_CLASS s = static_cast<jstring>(jniEnv->GetObjectArrayElement(arr, i));
				out.push_back(s.toString());																// "src \t Target \t Name"
			}
		});

		return out;
	}
#endif

	// Non-Android, or Android with real FS path
	QStringList out;
	QDir root(path);
	if(!root.exists())
		return out;

	// Build list of directories to scan
	QVector<QDir> scan;
	scan << root;

	// If user picked "Data", also scan ../Maps and ../Mp3 (if present)
	if(root.dirName().compare(QLatin1String("Data"), Qt::CaseInsensitive) == 0)
	{
		QDir parent = root;
		if(parent.cdUp())
		{
			addIfExists(scan, parent, "Maps");
			addIfExists(scan, parent, "Mp3");
		}
	}

	// Depth-first traversal on each directory; classify by extension
	for(const QDir &dir : scan)
	{
		QDirIterator it(dir.absolutePath(), QDir::Files | QDir::Readable | QDir::NoSymLinks, QDirIterator::Subdirectories);

		while(it.hasNext())
		{
			const QString filePath = it.next();
			const QFileInfo file(filePath);
			const QString target = classifyTargetByExt(file.fileName());
			if(target.isEmpty())
				continue;

			out.push_back(filePath + QLatin1Char('\t') + target + QLatin1Char('\t') + file.fileName());	// "src \t Target \t Name"
		}
	}

	return out;
}

void sendFileToApp(QString path)
{
#if defined(VCMI_ANDROID)
	// delegate to Android activity which will copy to cache and share via FileProvider
	auto jstr = QJNI_OBJECT_CLASS::fromString(path);
	RUN_ON_ANDROID_MAIN_THREAD([jstr]() /*mutable*/ {
		JNI_CALL_METHOD(QT_ACTIVITY, void, "shareFile", "(Ljava/lang/String;)V", jstr.object<jstring>());
		//return QVariant();
	});
#elif defined(VCMI_IOS)
	// use iOS share sheet
	iOS_utils::shareFile(path.toStdString());
#else
	Q_UNUSED(path);
#endif
}

QString findSteamGameInstallDir(const QString & appId, const QString & fallbackInstallDir)
{
#ifdef VCMI_WINDOWS
	// Steam does not store per-game install paths in the registry - only the Steam client path.
	// The actual game location has to be resolved via the library folders list and the game's own manifest file.
	QVector<QPair<QString, QString>> steamRegKeys = {
		{ "HKEY_CURRENT_USER\\Software\\Valve\\Steam",               "SteamPath"   },
		{ "HKEY_LOCAL_MACHINE\\SOFTWARE\\Valve\\Steam",              "InstallPath" },
		{ "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Valve\\Steam", "InstallPath" },
	};

	QString steamPath;
	for(auto & regKey : steamRegKeys)
	{
		steamPath = QSettings(regKey.first, QSettings::NativeFormat).value(regKey.second).toString();
		if(!steamPath.isEmpty())
			break;
	}

	if(steamPath.isEmpty())
		return {};

	// Steam library folders can be spread across multiple drives - collect all of them
	QVector<QString> libraryPaths = { steamPath };
	QFile libraryFoldersFile(steamPath + "/steamapps/libraryfolders.vdf");
	if(libraryFoldersFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QRegularExpression pathRegex("\"path\"\\s*\"([^\"]+)\"");
		auto matches = pathRegex.globalMatch(QString::fromUtf8(libraryFoldersFile.readAll()));
		while(matches.hasNext())
			libraryPaths.push_back(matches.next().captured(1).replace("\\\\", "/"));
	}

	for(const QString & libraryPath : libraryPaths)
	{
		QFile manifestFile(libraryPath + "/steamapps/appmanifest_" + appId + ".acf");
		if(!manifestFile.open(QIODevice::ReadOnly | QIODevice::Text))
			continue;

		QString installDir = fallbackInstallDir;
		QRegularExpression installDirRegex("\"installdir\"\\s*\"([^\"]+)\"");
		auto installDirMatch = installDirRegex.match(QString::fromUtf8(manifestFile.readAll()));
		if(installDirMatch.hasMatch())
			installDir = installDirMatch.captured(1);

		if(installDir.isEmpty())
			continue;

		QString candidate = libraryPath + "/steamapps/common/" + installDir;
		if(QDir(candidate).exists())
			return candidate;
	}
#else
	Q_UNUSED(appId);
	Q_UNUSED(fallbackInstallDir);
#endif
	return {};
}

#ifdef VCMI_ANDROID
QString getInstallerPackageName()
{
	if(!QT_ACTIVITY.isValid())
		return {};

	return QJNI_OBJECT_CLASS::callStaticObjectMethod(
		"eu/vcmi/vcmi/util/FileUtil",
		"getInstallerPackageName",
		"(Landroid/content/Context;)Ljava/lang/String;",
		QT_ACTIVITY.object()
	).toString();
}
#endif

}
