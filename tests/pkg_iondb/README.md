# Native Fat Files System Deletion
rm riot_fatfs_disk.tar.gz && rm bin/riot_fatfs_disk.img 

# Native Fat Files System Creation
make compressed-image && make image

# Native Build and run main application
BUILD_IN_DOCKER=1 make all flash term