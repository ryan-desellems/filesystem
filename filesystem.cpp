#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<time.h>
using namespace std;

void    openFileSystem(fstream &fileSystemToOpen);
bool    checkEmpty(fstream &fileToCheck);
void    displayUserMenu();
void    buildFileSystem(fstream &infileSystem);
void    createStoreFile(fstream &infileSystem);
void    deleteFile();
void    renameFile();
void    openReadFile();
void    createFile();
void    listFile();
bool    verifySelection();
bool    isUnique(fstream &infileSystem, string userFileName);
string  getInputText();
int     locateFreeSpace(fstream &infileSystem, int contentLength);
bool    writeFileToLocation(fstream &infileSystem, int posInMemTable,
                            int contentLength, string content);
void    writeToFAT(fstream &infileSystem, string fileName, int startPos, int contentLength)



const int MAX_FILE_NAME     = 10;                       // File name is 10 chars
const int MAX_FILE_SIZE     = 100;                      // Content of file is 100 chars
const int STANDARD_MEM_SIZE = 2;                        // Digits detailing mem space are 2
const int FCB_SIZE          = 20;                       // Total mem of FCB is 20
const int NUM_FCB           = 20;                       // Maximum of 20 File Control Blocks
const int FAT_SIZE          = NUM_FCB * FCB_SIZE;       // 20 FCB * 20 char in size = 400
const int MEM_TABLE_START   = FAT_SIZE;                 // free mem table starts after FAT ends
const int BLOCK_SIZE        = 10;                       // A block is 10 chars
const int MEMORY_SIZE       = MAX_FILE_SIZE * NUM_FCB;  // Each file is max 100 bytes, Max 20 files
const int NUM_MEM_BLOCKS    = NUM_FCB * BLOCK_SIZE;     // Each block of mem is 10, 20 files, 200 possile blocks
const int MEMORY_START      = NUM_MEM_BLOCKS + FAT_SIZE;// 200 bytes for mem blocks, 400 bytes for FAT, so 600ish

struct {

    string fileName;        // 10 chars
    string timeCreated;     // 5 chars
    int    startPos;        // 2 chars
    int    fileSize;        // 2 chars 

} FileControlBlock;

//==========================================================================================
int main()
{
    fstream fileSystem;
    openFileSystem(fileSystem);
    if(!fileSystem.is_open())
    {
        cout << "Not opened here!";
    }

    while(1)
    {
        displayUserMenu();
        int choice;
        cin >> choice;
        switch(choice)
        {
            case 1:
                createStoreFile(fileSystem);
                break;
            case 2:
                deleteFile();
                break;
            case 3:
                renameFile();
                break;
            case 4:
                openReadFile();
                break;
            case 5:
                createFile();
                break;
            case 6:
                listFile();
                break;
            case 7:
                fileSystem.close();
                exit(1);
                break;
            default:
                cout << "Invalid selection. Terminating.";
                fileSystem.close();
                exit(1);
        }
    }
    fileSystem.close();
    return 0;
}
//==========================================================================================
void openFileSystem(fstream &fileSystemToOpen)
{
    bool makeFile = false;

    fileSystemToOpen.open("filesystem.txt", fstream::in | fstream::out);
    if(!fileSystemToOpen.is_open())
    {
        fileSystemToOpen.close();
        ofstream ofs("filesystem.txt");
        ofs.close();
        makeFile = true;
    }
    if(makeFile)
    {
        fileSystemToOpen.open("filesystem.txt", fstream::in | fstream::out);
        if(!fileSystemToOpen.is_open())
        {
            cout << "File system could not be created. Terminating.\n" << endl;
            exit(1);
            fileSystemToOpen.close();
        }
    }

    if(checkEmpty(fileSystemToOpen))
    {    
        buildFileSystem(fileSystemToOpen);
    }
}
//==========================================================================================
bool checkEmpty(fstream &fileToCheck)
{
    fileToCheck.seekg(0,ios::end);      // go to end of file
    if(fileToCheck.tellg() == 0)        // is file pos still at the start?
    {
        return true;
    }
    return false;
}
//==========================================================================================
void buildFileSystem(fstream &infileSystem)
{
    cout << "[Building file system.]";
    string emptyFCB;
    emptyFCB.append(FAT_SIZE, ' ');              // Create a string representing the FCBs
    infileSystem << emptyFCB;                    // write to file, using space as placeholder


    string memoryTable;                          
    memoryTable.append(NUM_MEM_BLOCKS,'0');      // Create a string represent the free mem table
    infileSystem << memoryTable;                 // write to file, using zero as a place holder

    string freeMemory;
    freeMemory.append(MEMORY_SIZE,' ');          // Use spaces to allocate all the memory needed for the file system.
    infileSystem << freeMemory;


    if(infileSystem.tellg() != 0)                 // if file is no longer empty, 
    {
        cout << "\n[File system built.]\n";
        return;                                   // Only continue if build success.
    }
    cout << "[File system build unsuccessful. Terminating.]\n";
    infileSystem.close();
    exit(1);
}
//==========================================================================================
void displayUserMenu()
{
    cout << "[File management system]" << endl;     // Print the menu of options
    cout << "________________________\n";
    cout << "Please choose an option.\n\n";
    cout << "1). Create/Store  \n";
    cout << "2). Delete        \n";
    cout << "3). Rename        \n";
    cout << "4). Open/Read     \n";
    cout << "5). Create        \n"; 
    cout << "6). List          \n";
    cout << "7). Exit          \n";
    cout << endl;
}
//==========================================================================================
void createStoreFile(fstream &infileSystem)
{
    char   ans;

    if(!verifySelection())                                  // Make sure user intended this option
    {
        return;
    }
    system("CLS");

    cout << "Please enter file name: \n";                   // get file name
    cout << "[Max file name size is 10]\n"; 
    string userFile = getInputText();
    system("CLS");


    while((userFile.length() > MAX_FILE_NAME ||                        
           userFile.length() == 0) ||
           !isUnique(infileSystem,userFile))                // while the file name is invalid
    {
        cout << "Your file name is invalid.\n";             // prompt for a valid file name
        cout << "Please enter file name: \n";
        cout << "[Max file name size is 10 chars.]\n";
        userFile = getInputText();
        system("CLS");

    }

    cout << "Please enter file content: \n";                // get file content
    cout << "[Max file size is 100 chars.]\n";
    string userContent = getInputText();
    system("CLS");

    while(userContent.length() > MAX_FILE_SIZE || userContent.length() == 0)
    {
        cout << "Your content is invalid.\n";               // while file content is invalid
        cout << "Please enter file content: \n";            // prompt for valid file content
        cout << "[Max file size is 100 chars.]\n";
        userContent = getInputText();
        system("CLS");

    }

    int contentSize = userContent.length();                         // get the length of the content
    cout << "FP at : " << infileSystem.tellg() << endl;
    int freeSpacePos = locateFreeSpace(infileSystem,contentSize);   // get the first block of the empty blocks to write to
    cout << "FP at : " << infileSystem.tellg() << endl;

    if(freeSpacePos == -1)                                          // if -1, that means no space available
    {
        char temp;                                                  // notify user and ask them to confirm

        cout << "Not enough space for the file. Press any key to return.\n";
        cin >> temp;
        return;
    }

    if(!writeFileToLocation(infileSystem, freeSpacePos, contentSize, userContent))
    {
        cout << "Critical error writing to file. Terminating.\n";
        infileSystem.close();
        exit(1);
    }

    writeToFAT(infileSystem,userFile,freeSpacePos,userContent.length());
//○ Decide if we have space for the file, locate location to place file
//○ Copy file into location
//○ Remove location from Free Allocation Table
//○ Create a entry in the FAT
//■ File Control Block: filename, starting location, creation time, size

    cout << "Created [" << userFile << "]\n";

}
//===========================================================================================
void deleteFile()
{

}
//===========================================================================================
void renameFile()
{

}
//===========================================================================================
void openReadFile()
{

}
//===========================================================================================
void createFile()
{

}
//===========================================================================================
void listFile()
{

}
//===========================================================================================
bool verifySelection()
{
    char ans;

    system("CLS");
    cout << "Create File? [Y / N]\n";
    cin >> ans;
    cin.ignore(999, '\n');
    if(ans != 'Y' && ans != 'y')
    {
        return false;
    }
    return true;
}
//============================================================================================
bool isUnique(fstream &infileSystem, string userFileName)
{
    int  curPos = 0;
    char fileName[MAX_FILE_NAME];                                   // create a c-string for the file name to read into
    while(curPos < FAT_SIZE)
    {
        infileSystem.seekg(curPos);                                 // move cursor to curpos
        if(infileSystem.peek() != ' ')                              // if the name isnt empty
            infileSystem.read(fileName, (sizeof(fileName) - 1));    // read the name
            string tmp(fileName);                                   // convert char array to string
            if(tmp == userFileName)                                 // file name is same as user file name
            {
                cout << "Your file name is not unique.\n";
                return false;                                       // not unique
            }
        curPos += 20;                                               // next file name starts 20 char after 
    }
    return true;
}
//============================================================================================
string getInputText()
{
    string inText;

    getline(cin, inText);
    cout << endl;
    return inText;
}
//============================================================================================
int locateFreeSpace(fstream &infileSystem, int contentLength)
{
    int  posMarker = 0;
    int  zCounter  = 0;
    int  numBlocksNeeded = (contentLength + 10 - 1) / 10;   // always round integer up as full block needed, fragmentation -> :*( 
    char bitValue[NUM_MEM_BLOCKS];                          // create an array to read the mem table into

    infileSystem.seekg(MEM_TABLE_START);                    // set the file ptr to the start of free alloc. table  
    infileSystem.read(bitValue,NUM_MEM_BLOCKS);
    while((posMarker < sizeof(bitValue)))                   // while pos marker is less than num of memblocks
    {
        cout << bitValue[posMarker] << endl;
        if(bitValue[posMarker] == '0')                      // if the block is empty
        {   
            zCounter++;                                     // count consecutive zeros, which mark open blocks
            if (zCounter >= numBlocksNeeded)                // if enough memory exists
            {
                return posMarker;                           // return the block which is empty            
            }
        }
        else
        {
            zCounter = 0;                                   // reset zero counter on nonconsecutive zeroes
        }
        posMarker++;                                        // check next spot
    }
    return -1;                                              // if all spots are full, return fail
}
//============================================================================================
bool writeFileToLocation(fstream &infileSystem, int posInMemTable,
                         int contentLength, string content)
{
    int  numBlocksNeeded = (contentLength + BLOCK_SIZE- 1) / BLOCK_SIZE;             // always round integer up as full block needed, fragmentation -> :*( 

    infileSystem.seekg(MEMORY_START + ((posInMemTable)*BLOCK_SIZE));  // start of mem table, plus blocks to get to file spot
    infileSystem.write(content.c_str(),content.length());    
    infileSystem.seekg(MEM_TABLE_START + posInMemTable);

    string ones;
    ones.append('1',numBlocksNeeded);
    infileSystem.write(ones.c_str(),ones.length()-1);
    if(!infileSystem)
    {
        cout << "Wahr?";
    }

    return true;
}
//============================================================================================
void writeToFAT(fstream &infileSystem, string fileName, int startPos, int contentLength)
{

}
//============================================================================================
