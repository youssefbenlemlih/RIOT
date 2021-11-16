# Information
These tests are made for the iondb package integration.
It should show the usage of the key features.
The tests use the fat filesystem which could be change to another filesystem because iondb uses the vfs layer.

The following is tested:
- TEST01: Create, Close, Open, Delete of a Dictionary
- TEST02: Read, Insert, Read, Delete, Read of Keys of test02Keys and test02Values
- TEST03: Insert Keys, Close Dictionary, Open Dictionary, Read Keys of test03Keys and test03Values
- TEST04: Update, Read, Delete, Insert, Update, Read of Keys of test04Keys, test04Values and test04UpdatedValues
- TEST05: Cursor Equality
- TEST06: Cursor Range
- TEST07: Cursor All Records
- TEST08: Testing Duplicates with Read and Cursor using test08Keys keys and test08Values values
- TEST09: Testing config_t usage and the manipulation
- TEST10: Testing byte arrays as values and use test02Keys

# Test Usage
If you want to test iondb on native you should first create an image on your local machine (Step 0).
After that you can build and run the application (Step 1).
The Application uses the shell. When the shell is loaded type "s" to start the tests.

Troubleshooting: 
- Sometimes the tests throw an "[MOUNT]: NOT SUCCESSFULL" error.
    - this could be resolved by running (Step 0.1) and (Step 0) afterwards.

- Sometimes the compilation fails
    - try to delete the bin/"your board (native etc.)" folder 

## 0. Native Fat Files System Creation
make compressed-image && make image

## 1. Native Build and run main application
BUILD_IN_DOCKER=1 make all flash term

## 0.1 Native Fat Files System Deletion
rm riot_fatfs_disk.tar.gz && rm bin/riot_fatfs_disk.img 