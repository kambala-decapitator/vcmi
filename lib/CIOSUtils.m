/*
 * CIOSUtils.m, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

#import "CIOSUtils.h"

@import Foundation;

static NSString *standardPathNative(NSSearchPathDirectory directory)
{
	return [NSFileManager.defaultManager URLForDirectory:directory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:NULL].path;
}
static const char *standardPath(NSSearchPathDirectory directory) { return standardPathNative(directory).fileSystemRepresentation; }

const char *ios_documentsPath() { return standardPath(NSDocumentDirectory); }
const char *ios_cachesPath() { return standardPath(NSCachesDirectory); }

NSURL *sharedContainerURL()
{
    static NSURL *sharedPathURL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __auto_type bundleID = NSBundle.mainBundle.bundleIdentifier;
        __auto_type lastDotPos = [bundleID rangeOfString:@"." options:NSBackwardsSearch].location;
        __auto_type groupID = [NSString stringWithFormat:@"group.%@.vcmi", [bundleID substringToIndex:lastDotPos]];
        sharedPathURL = [NSFileManager.defaultManager containerURLForSecurityApplicationGroupIdentifier:groupID];
    });
    return sharedPathURL;
}
NSURL *sharedGameDataURL() { return [sharedContainerURL() URLByAppendingPathComponent:@"GameData"]; }
const char *ios_sharedDataPath() { return sharedGameDataURL().fileSystemRepresentation; }

#if TARGET_OS_SIMULATOR
const char *ios_hostApplicationSupportPath()
{
    static NSString *applicationSupportPath;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        __auto_type cachesPath = standardPathNative(NSCachesDirectory);
        __auto_type afterMacOsHomeDirPos = [cachesPath rangeOfString:@"Library/Developer"].location;
        NSCAssert(afterMacOsHomeDirPos != NSNotFound, @"simulator directory location is not under user's home directory: %@", cachesPath);
        applicationSupportPath = [[cachesPath substringToIndex:afterMacOsHomeDirPos] stringByAppendingPathComponent:@"Library/Application Support/vcmi"].stringByResolvingSymlinksInPath;
    });
    return applicationSupportPath.fileSystemRepresentation;
}
#endif

const char *ios_bundlePath() { return NSBundle.mainBundle.bundlePath.fileSystemRepresentation; }
const char *ios_frameworksPath() { return NSBundle.mainBundle.privateFrameworksPath.fileSystemRepresentation; }

const char *ios_bundleIdentifier() { return NSBundle.mainBundle.bundleIdentifier.UTF8String; }
