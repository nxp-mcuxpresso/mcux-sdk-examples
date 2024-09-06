#!/bin/sh

#set path variable for current directory
DIR_PATH=$(pwd)
OT_NXP_PATH="$DIR_PATH/ot-nxp"
NXP_IW612_SDK_ROOT="$OT_NXP_PATH/third_party/github_sdk"
PATCH_PATH="$DIR_PATH/TriRadioCoex_Patch"
ARMGCC_Path="$NXP_IW612_SDK_ROOT/examples/evkbmimxrt1170/coex_examples/coex_cli/cm7/armgcc/"
LINKER_PATH="$ARMGCC_Path/linker"
#====================================================================================================
#Usage
#pass OT tag and Open thread tag as an argument
#====================================================================================================
usage()
{
	echo "\n*********************************************************************\n"
	echo "Usage: Pass OT tag as 1st argument and Openthread tag as 2nd argument \n"
	echo "*********************************************************************\n"
	exit 1
}
 
#=====================================================================================================
#Clone ot-nxp repo for OT related code base
#=====================================================================================================
Git_Clone_OT_repo()
{
	git clone ssh://git@bitbucket.sw.nxp.com/connint/ot-nxp.git
	#OT tag available with the release
	cd $OT_NXP_PATH
	git checkout $OT_tag
	git submodule update --init
	cd $OT_NXP_PATH/openthread
	#Openthread tag available with the release
        git checkout $OpenThread_tag
}

#=====================================================================================================
# Apply West patch,west init and west update the required repos
#=====================================================================================================
West_Init()
{
	
	#Apply the patch of west file to update the revision and repos
	West_Patch="$PATCH_PATH/west_update.patch"
	cd $NXP_IW612_SDK_ROOT
	echo "$West_Patch"
	if [ -e $West_Patch ]; then 
		git apply $West_Patch
	else
		echo "west_update patch not found at mentioned location $PATCH_PATH"
		exit 1
		
	fi
	#Apply the patch of west file to update the revision and repos
	west init -l manifest --mf west.yml
    west update
}
#=====================================================================================================
# Apply CMakeFile,Linker and Flags.cmake file patch
#=====================================================================================================
CMake_Apply_Patch()
{
	#Apply the patch for CMakeFile,flags.cmake and replace linker file which consist of changes required for tri-radio coex app
	CMake_Patch="$PATCH_PATH/CMake_Flagcmake.patch"
	Linker_File="$PATCH_PATH/MIMXRT1176xxxxx_cm7_flexspi_nor.ld"
	Littlefs_Patch="$PATCH_PATH/littlefs_pl.patch"
	cd "$NXP_IW612_SDK_ROOT/examples"
	if [ -e $CMake_Patch ]; then
		echo "Applying the GIT patch $CMake_Patch"
		git apply $CMake_Patch
	else
		echo "Cmake patch not found at mentioned location $PATCH_PATH"
		exit 1
	fi

	if [ -e $Linker_File ]; then
		echo "Replace existing linker file with updated one "
	        cp $Linker_File $LINKER_PATH	
	else
		echo "Linker file not found at the location $Linker_File"
	fi

	if [ -e $Littlefs_Patch ]; then
	    cd "$NXP_IW612_SDK_ROOT/middleware/wireless/ethermind"
		echo "Applying the GIT patch $Littlefs_Patch"
	        git apply $Littlefs_Patch	
	else
		echo "littlefs patch not found at the location $Littlefs_File"
	fi
	
}
#=======================================================================================================
#Execute bootstrap script for setup the compilation environment
#=======================================================================================================
Setup_Compilation_Env()
{

	cd $OT_NXP_PATH 
	chmod 777 script/bootstrap
	dos2unix script/bootstrap
	chmod 777 openthread/script/bootstrap
	dos2unix openthread/script/bootstrap
	./script/bootstrap
}
#========================================================================================================
#compile OT binaries for evkb1170
#========================================================================================================
Compile_OT_Binaries()
{
	#check if exist then remove
	cd $OT_NXP_PATH
	CMakeCache="CMakeCache.txt"
	CMakeFile_folder="CMakeFiles"
	Lib_folder=build_rt1170
	Debug_File="$DIR_PATH/Ot_compilation_debug.txt"
	if [ -e $CMakeCache ]; then
		rm $CMakeCache
	fi
	if [ -e $CMakeFile_folder ]; then
		rm -rf $CMakeFile_folder
	fi
	if [ -e $Lib_folder ]; then
		rm -rf $Lib_folder
	fi

	chmod 777 script/build_rt1170
	dos2unix script/build_rt1170
	./script/build_rt1170  iwx12_spi -DOT_NXP_BUILD_APP_AS_LIB=ON -DBOARD_APP_UART_INSTANCE=2 -DOT_APP_CLI_FREERTOS_IPERF=OFF > $Debug_File
	
}
#========================================================================================================
#Compile tri-radio coex app for EVKB1170
#========================================================================================================
Compile_TriRadio_Coex()
{

	export ARMGCC_DIR=/opt/gcc-arm-none-eabi-9-2020-q2-update/
	ARMGCC_Path="$NXP_IW612_SDK_ROOT/examples/evkbmimxrt1170/coex_examples/coex_cli/cm7/armgcc/"
	cd $ARMGCC_Path
	chmod 777 build_flexspi_nor_release.sh
	dos2unix build_flexspi_nor_release.sh
	./build_flexspi_nor_release.sh

}
#========================================================================================================
 if [ $# -ne 2 ] ; then
	usage
 else
	OT_tag=$1
	OpenThread_tag=$2
 fi


Git_Clone_OT_repo
West_Init
CMake_Apply_Patch
Setup_Compilation_Env
Compile_OT_Binaries
Compile_TriRadio_Coex
