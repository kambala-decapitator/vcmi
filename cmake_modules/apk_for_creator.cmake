set(apkRelativeDir "build/outputs/apk/${CREATOR_BUILD_TYPE}")
set(apkCreatorDir "${CMAKE_BINARY_DIR}/android-build/${apkRelativeDir}")

file(MAKE_DIRECTORY ${apkCreatorDir})
file(CONFIGURE OUTPUT "${CMAKE_BINARY_DIR}/asd.cmake" CONTENT [=[
	set(apkVcmiDir "../../../../vcmi-app/@apkRelativeDir@")
	file(CREATE_LINK "${apkVcmiDir}/vcmi-@CREATOR_BUILD_TYPE@.apk" "android-build-@CREATOR_BUILD_TYPE@.apk"
		COPY_ON_ERROR SYMBOLIC
	)
	file(CREATE_LINK "${apkVcmiDir}/output-metadata.json" "output-metadata.json"
		COPY_ON_ERROR SYMBOLIC
	)
]=] @ONLY)
# file(WRITE "${CMAKE_BINARY_DIR}/asd.cmake" "
# 	set(apkVcmiDir \"../../../../vcmi-app/${apkRelativeDir}\")
# 	file(CREATE_LINK \"\${apkVcmiDir}/vcmi-${CREATOR_BUILD_TYPE}.apk\" \"android-build-${CREATOR_BUILD_TYPE}.apk\"
# 		COPY_ON_ERROR SYMBOLIC
# 	)
# 	file(CREATE_LINK \"\${apkVcmiDir}/output-metadata.json\" \"output-metadata.json\"
# 		COPY_ON_ERROR SYMBOLIC
# 	)
# ")
execute_process(
	COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/asd.cmake"
	WORKING_DIRECTORY ${apkCreatorDir}
)
# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E create_symlink
#         "../../../../vcmi-app/{apkRelativeDir}/vcmi-${CREATOR_BUILD_TYPE}.apk"
#         "android-build-${CREATOR_BUILD_TYPE}.apk"
#     COMMAND ${CMAKE_COMMAND} -E create_symlink
#         "../../../../vcmi-app/{apkRelativeDir}/output-metadata.json"
#         "output-metadata.json"
#     WORKING_DIRECTORY ${apkCreatorDir}
# )
