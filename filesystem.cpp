//Name:      Ryan DeSellems
//Date:      October 30th, 2022
//Course:    COMP 3340 - Saas
//Project:   File System within a text file

#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<ctime>
#include<time.h>
#include<unistd.h>
#include<stdio.h>

using namespace std;

// Function party! Everyone's invited!
void    openFileSystem(fstream &fileSystemToOpen);
bool    checkEmpty(fstream &fileToCheck);
void    displayUserMenu();
void    buildFileSystem(fstream &infileSystem);
void    createStoreFile(fstream &infileSystem);
void    deleteFile(fstream &infileSystem);
void    renameFile(fstream &infileSystem);
void    openFile(fstream &infileSystem);
void    listFile(fstream &infileSystem);
bool    verifySelection();
string  getFileName(string operationDesc);
string  getFileContent();
bool    fileExists(fstream &infileSystem, string userFileName);
bool    startsWithSpaces(string stringToCheck);
string  getInputText();
bool    fileSpotAvailable(fstream &fileSystemIn);
int     locateFreeSpace(fstream &infileSystem, int contentLength);
bool    writeFileToLocation(fstream &infileSystem, int posInMemTable,int contentLength, string content);
void    writeToFAT(fstream &infileSystem, string fileName, int startPos, int contentLength);
bool    createFATEntry(fstream &infileSystem, string fileName, int startPos, int contentLength,int fcbWritePos);
string  getFileControlBlock(fstream &infileSystem, string fileBlockToGet);
void    deleteFomFAT(fstream &infileSystem, int nameLocation);
void    deleteFromMemTable(fstream &infileSystem, int firstBlock, int numBlocks);
void    deleteFromMemory(fstream &infileSystem, int firstBlock, int numBlocks);
void    displaySuccessScreen(string operation);
string  readFileContent(fstream &infileSystem, string fcbInfo);
void    handleDestroyF(fstream &fileSysToDestroy);

// Global variables
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

//     Layout of the FCB (In hindsight, I probably should have also used globals for these. RIP)
//     {
//         string fileName;       - 10 chars
//         string timeCreated;    - 5  chars
//         int    startPos;       - 3  chars
//         int    fileSize;       - 2  chars 
//                                - --------
//         total length           - 20 chars

//          side note: some functions return the location of the fcb after the 20 chars,
//                     meaning it is 23 chars in certain situations, but never in the file.
//                     EX. getFileControlBlock()

//     }

//==========================================================================================
int main()
{
    fstream fileSystem;
    char    choice;

    openFileSystem(fileSystem);
    if(!fileSystem.is_open())
    {
        cout << "Not opened here!";
    }

    while(1)
    {
        displayUserMenu();
        cin >> choice;
        cin.ignore(512,'\n');
        switch(choice)
        {
            case '1':
                createStoreFile(fileSystem);
                break;
            case '2':
                deleteFile(fileSystem);
                break;
            case '3':
                renameFile(fileSystem);
                break;
            case '4':
                openFile(fileSystem);
                break;
            case '5':
                listFile(fileSystem);
                break;
            case '6':
                fileSystem.close();
                exit(1);
                break;
            case '|':
                handleDestroyF(fileSystem);
                break;
            default:
                break;
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
    cout << "5). List          \n";
    cout << "6). Exit          \n";
    cout << endl;
}
//==========================================================================================
void createStoreFile(fstream &infileSystem)
{
    int     contentSize;
    int     freeSpacePos;
    char    ans;
    string  userFile;
    string  userContent;

    system("CLS");
    if(!verifySelection())                                  // Make sure user intended this option
    {
        return;
    }
    system("CLS");

    userFile = getFileName("to create");

    while((userFile.length() > MAX_FILE_NAME ||                        
           userFile.length() == 0)           ||
           fileExists(infileSystem,userFile) ||
           startsWithSpaces(userFile))                   // while the file name is invalid
    {
        cout << "Your file name is invalid.\n";             // prompt for a valid file name
        cout << "[It must be less than 10 chars, can't start with a space, and can't exist already.]\n\n";            
        userFile = getFileName("to create");
        system("CLS");

    }                

    if(!fileSpotAvailable(infileSystem))                             
    {
        cout << "Too many files, unable to create new file.\n";
        return;
    }

    userContent = getFileContent();

    contentSize = userContent.length();                             // get the length of the content
    freeSpacePos = locateFreeSpace(infileSystem,contentSize);       // get the first block of the empty blocks to write to

    if(freeSpacePos == -1)                                          // if -1, that means no space available
    {
        cout << "Not enough space for the file.\n";
        return;
    }

    if(!writeFileToLocation(infileSystem, freeSpacePos, contentSize, userContent))
    {
        cout << "Critical error writing to file. Terminating.\n";
        infileSystem.close();
        exit(1);
    }

    writeToFAT(infileSystem,userFile,freeSpacePos,userContent.length());

    displaySuccessScreen("File creation");
}
//===========================================================================================
string getFileName(string operationDesc)
{
    string userFileName;

    cout << "Please enter name of file " << operationDesc << ": \n";                   // get file name
    cout << "[Max file name size is 10]\n"; 
    userFileName = getInputText();
    system("CLS");

    return userFileName;   
}
//===========================================================================================
string getFileContent()
{
    string userContentToReturn;

    cout << "Please enter file content: \n";                // get file content
    cout << "[Max file size is 100 chars.]\n";
    userContentToReturn = getInputText();
    system("CLS");

    while(userContentToReturn.length() > MAX_FILE_SIZE || userContentToReturn.length() == 0)
    {
        cout << "Your content is invalid.\n";               // while file content is invalid
        cout << "Please enter file content: \n";            // prompt for valid file content
        cout << "[Max file size is 100 chars.]\n";  
        userContentToReturn = getInputText();
        system("CLS");

    }

    return userContentToReturn;
}
//===========================================================================================
bool startsWithSpaces(string stringToCheck)
{
    if(stringToCheck[0] == ' ')
    {
        return true;                // just check if first char is space, return accordingly
    }
    return false;
}
//===========================================================================================
void deleteFile(fstream &infileSystem)
{
    string fcbString;
    int    firstBlockOfContent;
    int    numBlocksOccupied;
    int    fileNameLocation;

    system("CLS");
    if(!verifySelection())                                  // Make sure user intended this option
    {
        return;
    }
    system("CLS");
    string fileToDelete = getFileName("to delete");

    while(!fileExists(infileSystem,fileToDelete))           // while file doesn't exist
    {
        cout << "File doesn't exist.\n";                    // see whether to return to menu or try new file name
        cout << "Return? [Y / N]\n";
        char temp;
        cin >> temp;
        cin.ignore(512,'\n');
        if(temp == 'Y' || temp == 'y')                      // if y, go to menu
        {
            return;
        }
        fileToDelete = getFileName("to delete");           // else try new file name
    }

    fcbString = getFileControlBlock(infileSystem,fileToDelete);

    if(fcbString.compare("NONE") == 0)
    {
        cout << "Error in locating File Control Block.\n";
        return;
    }

    firstBlockOfContent = stoi(fcbString.substr(15,3));  // Get int values from formatted return string
    numBlocksOccupied   = stoi(fcbString.substr(18,2));  // in getFileControlBlock()
    fileNameLocation    = stoi(fcbString.substr(20,3));

    deleteFomFAT(infileSystem,fileNameLocation);
    deleteFromMemTable(infileSystem,firstBlockOfContent,numBlocksOccupied);
    deleteFromMemory(infileSystem,firstBlockOfContent,numBlocksOccupied);

    displaySuccessScreen("File deletion");

}
//============================================================================================
void renameFile(fstream &infileSystem)
{
    string fileToRename;
    string newFileName;
    string fileBlockOfFileToRename;
    int    fileToRenamePos;

    system("CLS");
    if(!verifySelection())                                              // Make sure user intended this option
    {
        return;
    }
    system("CLS");
    
    cout << "[FILE TO RENAME]\n\n";
    fileToRename = getFileName("to rename");

    while(!fileExists(infileSystem,fileToRename))                       // while file doesn't exist
    {
        cout << "File doesn't exist.\n";                                // see whether to return to menu or try new file name
        cout << "Return? [Y / N]\n";
        char temp;
        cin >> temp;
        cin.ignore(512,'\n');
        if(temp == 'Y' || temp == 'y')                                  // if y, go to menu
        {
            return;
        }
        system("CLS");
        cout << "[FILE TO RENAME]\n\n";
        fileToRename = getFileName("to rename");                                   // else try new file name
    }

    cout << "[NEW NAME OF FILE]\n\n";
    newFileName = getFileName("to be renamed's new name");
    while(fileExists(infileSystem,newFileName))                         // while file exists
    {
        cout << "File already exists.\n";                               // see whether to return to menu or try new file name
        cout << "Return? [Y / N]\n";
        char temp;
        cin >> temp;
        cin.ignore(512,'\n');
        if(temp == 'Y' || temp == 'y')                                  // if y, go to menu
        {
            return;
        }
        system("CLS");
        cout << "[NEW NAME OF FILE]\n\n";
        newFileName = getFileName("to be renamed's new name");                                   // else try new file name
    }

    fileBlockOfFileToRename = getFileControlBlock(infileSystem,fileToRename);
    fileToRenamePos = stoi(fileBlockOfFileToRename.substr(20,3));       // get the location of the file block to rename
    infileSystem.seekg(ios::beg + fileToRenamePos);
    newFileName = newFileName.append((10-newFileName.length()),' '); // force file to rename to 10 chars, probably should have made getFileName() do this. Too late.    
    infileSystem.write(newFileName.c_str(),MAX_FILE_NAME);             // overwrite old name with new

    displaySuccessScreen("File renaming");

}
//============================================================================================
void openFile(fstream &infileSystem)
{
    string fcbString;
    string fileToOpen;
    string contentToShow;
    int    firstBlockOfContent;
    int    numBlocksOccupied;
    int    fileNameLocation;

    system("CLS");
    if(!verifySelection())                                      // Make sure user intended this option
    {
        return;
    }
    system("CLS");
    fileToOpen = getFileName("to open");

    while(!fileExists(infileSystem,fileToOpen))               // while file doesn't exist
    {
        cout << "File doesn't exist.\n";                        // see whether to return to menu or try new file name
        cout << "Return? [Y / N]\n";
        char temp;
        cin >> temp;
        cin.ignore(512,'\n');
        if(temp == 'Y' || temp == 'y')                          // if y, go to menu
        {
            return;
        }
        fileToOpen = getFileName("to open");                           // else try new file name
    }

    fcbString = getFileControlBlock(infileSystem,fileToOpen);
    contentToShow = readFileContent(infileSystem,fcbString);
    cout << "Displaying content of " << fileToOpen << ".\n\n";
    cout << "________________________\n";
    cout << contentToShow << endl;
    cout << "________________________\n\n\n\n\n\n\n";
    system("pause");

}
//============================================================================================
void listFile(fstream &infileSystem)
{
    char fcbArray[FAT_SIZE];                                   

    infileSystem.seekg(ios::beg);
    infileSystem.read(fcbArray,FAT_SIZE);                      // read FAT table into char array

    string fcbString(fcbArray);                                 // make it a string
    
    system("CLS");
    if(!verifySelection())                                      // Make sure user intended this option
    {
        return;
    }
    system("CLS");

    cout << "File list: \n";
    cout << "________________________\n";

    for (int i = 0; i < FAT_SIZE; i += 20)
    {
        if(fcbString[i] != ' ')
        {
            string temp = fcbString.substr(i,FCB_SIZE-5);       // remove the start pos and content length, keep name and time
            temp.insert(10," Time created: ");                  // space out name and time;
            cout << temp << endl;                               // display it
        }
    }
    
    cout << "________________________\n";
    system("pause");
    system("CLS");
}
//============================================================================================
bool verifySelection()
{
    char ans;

    cout << "Are you sure? [Y / N]\n";                  // Gives user an option to return to main menu
    cin >> ans;
    cin.ignore(999, '\n');
    if(ans != 'Y' && ans != 'y')
    {
        return false;
    }
    return true;
}
//============================================================================================
bool fileExists(fstream &infileSystem, string userFileName)
{
    int  curPos = 0;
    char fileBlocks[FAT_SIZE];                                        // create a c-string for the FAT to be read into

    infileSystem.seekg(ios::beg);                                     // move cursor to start of file
    infileSystem.read(fileBlocks, (sizeof(fileBlocks)));              // read the file blocks into array, (minimizing mem access)

    userFileName = userFileName.append(10-userFileName.length(),' '); // always set string length to 10
    string filesString(fileBlocks);                                   // turn fileNames into a string so easier manipulation

    while(curPos < FAT_SIZE)
    {
        if(filesString[curPos] != ' ')
        {                                                              // if the name isnt empty (there's a space for the first pos)
            string temp(filesString,curPos,10);                        // get substring with file name (which will always be 10 char)
            if(temp.compare(userFileName) == 0)                        // file name is same as user file name
            {
                return true;                                           // the file exists
            }
        }
        curPos += 20;                                                  // next file name starts 20 char after 
    }
    return false;                                                      // if through array it doesn't exist
}
//============================================================================================
string getInputText()
{
    string inText;
    getline(cin, inText);  // just return text from user
    cout << endl;
    return inText;
}
//============================================================================================
bool fileSpotAvailable(fstream &fileSystemIn)
{
    char fatArray[FAT_SIZE];                                // array to read FAT into

    fileSystemIn.seekg(ios::beg);                           // go to beg of file
    fileSystemIn.read(fatArray, FAT_SIZE);                  // read FAT into fatArray
    for (int i = 0; i < FAT_SIZE; i += 20)                         // check 20 fat entries
    {
        if(fatArray[i] == ' ')                               // if first spot a space, available
        {
            return true;                                    
        }
    }
    return false;                                           // else there is no space available;
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
        if(bitValue[posMarker] == '0')                      // if the block is empty
        {   
            zCounter++;                                     // count consecutive zeros, which mark open blocks
            if (zCounter >= numBlocksNeeded)                // if enough memory exists
            {
                return (posMarker+1) - numBlocksNeeded;     // return the first block which is empty       
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
bool writeFileToLocation(fstream &infileSystem, int posInMemTable, int contentLength, string content)

{
    int  numBlocksNeeded = (contentLength + BLOCK_SIZE- 1) / BLOCK_SIZE;  // always round integer up as full block needed, fragmentation -> :*( 

    int writeStart = MEMORY_START + ((posInMemTable)*BLOCK_SIZE);

    infileSystem.seekg(writeStart);                                      // start of mem table, plus blocks to get to file spot
    infileSystem.write(content.c_str(),content.length());                // write content starting there
    infileSystem.seekg(MEM_TABLE_START + posInMemTable);                 // go to start of free mem

    string ones;
    ones.append(numBlocksNeeded,'1');                                    // create a string with a 1 for each block needed
    infileSystem.write(ones.c_str(),ones.length());                      // write it
    if(!infileSystem)
    {
        cout << "Wahr?";
    }

    return true;
}
//============================================================================================
void writeToFAT(fstream &infileSystem, string fileName, int startPos, int contentLength)
{
    char FCBarray[FAT_SIZE];

    infileSystem.seekg(0, ios::beg);                                        // reset fp to beginning
    infileSystem.read(FCBarray,FAT_SIZE);                                   // get FCB table into an array
    for (int i = 0; i < FAT_SIZE; i += 20)                                  // check start of each fcb
    {
        if(FCBarray[i] == ' ')                                              // space marks empty FCB
        {
            createFATEntry(infileSystem,fileName,startPos,contentLength,i);   // create FAT entry
            return;
        }
    }  
}
//============================================================================================
bool createFATEntry(fstream &infileSystem, string fileName, int startPos, int contentLength, int fcbWritePos)
{
    string      fcbToWrite;
    string      spString;
    string      clString;
    string      spStringPad;
    string      clStringPad;
    int         numBlocksNeeded;
    time_t      ltime;

    numBlocksNeeded = (contentLength + 10 - 1) / 10;    // always round integer up as full block needed, fragmentation -> :*( 

    time(&ltime); 

    string timeString(ctime(&ltime));
    timeString = timeString.substr(11,5);

    fileName = fileName.append((10-fileName.length()),' ');
    spString = to_string(startPos);
    clString = to_string(numBlocksNeeded);
    spStringPad = spStringPad.append(3-spString.length(),'0') + spString; //3 minus length
    clStringPad = clStringPad.append(2-clString.length(),'0') + clString; //2 minus length

    fcbToWrite = fileName + timeString + spStringPad + clStringPad;

    infileSystem.seekg(fcbWritePos);
    infileSystem.write(fcbToWrite.c_str(),20);

    return true;    
}
//============================================================================================
string getFileControlBlock(fstream &infileSystem, string fileBlockToGet)
{
    int  curPos = 0;
    char fileBlocks[FAT_SIZE];                                                          // create a c-string for the FAT to be read into

    infileSystem.seekg(ios::beg);                                                       // move cursor to start of file
    infileSystem.read(fileBlocks, (sizeof(fileBlocks)));                                // read the file blocks into array, (minimizing mem access)

    fileBlockToGet = fileBlockToGet.append(10-fileBlockToGet.length(),' ');             // always set string length to 10
    string filesString(fileBlocks);                                                     // turn fileNames into a string so easier manipulation

    while(curPos < FAT_SIZE)
    {
        if(filesString[curPos] != ' ')
        {                                                                               // if the name isnt empty (there's a space for the first pos)
            string temp(filesString,curPos,10);                                         // get substring with file name (which will always be 10 char)
            if(temp.compare(fileBlockToGet) == 0)                                       // file name is same as user file name
            {
                string returnString(filesString,curPos,20);                             // get substring with file block
                string locString    = to_string(curPos);                                // convert curPos to string
                string locStringPad = "";                                               // Apparently if I don't include this and try to append on the fly, the program crashes. Pretty sick.
                locStringPad = locStringPad.append(3-locString.length(),'0');           // force to length of 3
                return returnString + locStringPad + locString;                         // the file exists, return it
            }
        }
        curPos += 20;                                                                   // next file name starts 20 char after 
    }
    return "NONE";
}
//============================================================================================
void deleteFomFAT(fstream &infileSystem, int nameLocation)
{
    string deleteString;           

    deleteString = deleteString.append(FCB_SIZE,' ');           // 20 spaces to rewrite name slot
    infileSystem.seekg(ios::beg + nameLocation);                // go to file name in FAT Table
    infileSystem.write(deleteString.c_str(), FCB_SIZE);         // rewrite
}
//============================================================================================
void deleteFromMemTable(fstream &infileSystem, int firstBlock, int numBlocks)
{
    string deleteString;

    deleteString = deleteString.append(numBlocks,'0');       // numBlocks zeroes to rewrite name slot
    infileSystem.seekg(MEM_TABLE_START + firstBlock);        // get file cursor to start of mem to reset
    infileSystem.write(deleteString.c_str(), numBlocks);     // rewrite

}
//============================================================================================
void deleteFromMemory(fstream &infileSystem, int firstBlock, int numBlocks)
{
    string deleteString;

    deleteString = deleteString.append(numBlocks*BLOCK_SIZE,' ');               // a lot of spaces to rewrite name slot
    infileSystem.seekg(MEMORY_START + firstBlock*BLOCK_SIZE);                   // get file cursor to start of mem to reset
    infileSystem.write(deleteString.c_str(), deleteString.length());            // rewrite
}
//============================================================================================
void displaySuccessScreen(string operation)
{
    system("CLS");
    cout << "[" << operation << " was a success.]\n";
    system("pause");
    system("CLS");

}
//============================================================================================
string readFileContent(fstream &infileSystem, string fcbInfo)
{
    int    firstBlock;
    int    numBlocks;

    firstBlock  = stoi(fcbInfo.substr(15,3));   // Get int values from formatted in string
    numBlocks   = stoi(fcbInfo.substr(18,2)); 

    char readString[numBlocks*BLOCK_SIZE];

    infileSystem.seekg(MEMORY_START + firstBlock*BLOCK_SIZE);                   // get file cursor to start of mem to reset
    infileSystem.read(readString, numBlocks*BLOCK_SIZE); 

    string returnString(readString);
    return returnString;
     
}
//============================================================================================
 void handleDestroyF(fstream &fileSysToDestroy)
 { 
    system("CLS");
    cout << "Doing so will erase the filesystem.\n";
    if(!verifySelection())
    {
        return;
    }
    cout << "Confirm.\n";
    if(!verifySelection())
    {
        return;
    }
    fileSysToDestroy.close();
    remove("filesystem.txt");
    exit(0);

 }
 //===========================================================================================