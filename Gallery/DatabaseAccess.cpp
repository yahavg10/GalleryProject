#include <map>
#include <iostream>
#include <algorithm>
#include <io.h>
#include <string>
#include "ItemNotFoundException.h"
#include "sqlite3.h"
#include "DatabaseAccess.h"
#include <stdlib.h>
using std::string;
using std::to_string;
using std::strcat;
std::list<Album> albumsList;
Album myAlbum = Album();
User myuser = User(0, "default");
int key = 0;

bool DatabaseAccess::open() {
    int doesFileExist = _access("MyDB.sqlite", 0);
    int result = sqlite3_open("MyDB.sqlite", &db);

    if (result != SQLITE_OK) {
        db = nullptr;
        throw MyException("Problem opening gallery database.");
        return false;
    }

    if (doesFileExist == -1) {
        createTable("ALBUMS", "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL,CREATION_DATE TEXT NOT NULL, USER_ID INTEGER NOT NULL");
        createTable("USERS", "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL");
        createTable("PICTURES", "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL, CREATION_DATE TEXT NOT NULL, ALBUM_ID INTEGER TEXT NOT NULL");
        createTable("TAGS", "PICTURE_ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, USER_ID INTEGER NOT NULL");
    }

    return true;
}
void DatabaseAccess::createTable(std::string tableName, std::string tableParameters) {
    int result = 0;
    char* errMessage = nullptr;
    std::string sqlStatement = "CREATE TABLE " + tableName + " (" + tableParameters + "); ";
    const char* sqlCommand = sqlStatement.c_str();

    result = sqlite3_exec(db, sqlCommand, nullptr, nullptr, nullptr);

    if (result != SQLITE_OK) {
        throw MyException("Problem creating " + tableName + " table.");
    }
}
void DatabaseAccess::close()
{
    sqlite3_close(db);
    db = nullptr;
}

/*NEED TO FINISH*/int callback_id(void* data, int argc, char** argv, char** azColName)
{
    key = atoi(argv[0]);
    return 0;
}



int callback_print(void* data, int argc, char** argv, char** azColName)
{
    for (int i = 0; i < argc; i++)
    {
        std::cout << azColName[i] << " = " << argv[i] << " , ";
    }
    std::cout << std::endl;
    return 0;
}



void DatabaseAccess::createAlbum(const Album& album)
{
    std::string sqlStatementS = "INSERT INTO ALBUMS (NAME, CREATION_DATE,USER_ID) VALUES (\"" + album.getName() + "\", \"" + album.getCreationDate() + "\", \"" + to_string(album.getOwnerId()) + "\");";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    char* errMessage = nullptr;
    if (open())
    {
        int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
        if (res == SQLITE_MISUSE)
            std::cout << "Library used incorrectly" << std::endl;
        //close();
    }
}



void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
    if (doesAlbumExists(albumName, userId))
    {
        char* errMessage = nullptr;
        std::string sqlStatementS = "DELETE FROM ALBUMS WHERE NAME = \"" + albumName;
        sqlStatementS += "\" AND USER_ID = \"" + to_string(userId) + "\";";
        char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
        if (open())
        {
            int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
        }
    }
}



bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
    char* errMessage = nullptr;
    int doesExist = 999;
    std::string sqlStatementS = "SELECT EXISTS(SELECT 1 FROM ALBUMS WHERE NAME = \"" + albumName + "\");";
    //sqlStatementS += "\" AND USER_ID = \"" + to_string(userId) + "\");";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, &doesExist, &errMessage);
    doesExist = key;
    if (doesExist != 1)
    {
        return false;
    }
    else
        return true;
}



int callback_album(void* data, int argc, char** argv, char** azColName)
{
    for (int i = 0; i < argc; i++) {
        if (string(azColName[i]) == "NAME") {
            myAlbum.setName((argv[i]));
        }
        else if (string(azColName[i]) == "CREATION_DATE") {
            myAlbum.setCreationDate(argv[i]);
        }
        else if (string(azColName[i]) == "USER_ID") {
            myAlbum.setOwner(atoi(argv[i]));
        }
    }
    return 0;
}

int callback_user(void* data, int argc, char** argv, char** azColName)
{
    for (int i = 0; i < argc; i++) {
        if (string(azColName[i]) == "NAME") {
            myuser.setName(argv[i]);
        }
        else if (string(azColName[i]) == "ID") {
            myuser.setId(atoi(argv[i]));
        }
        std::cout << argv[i];
    }
    return 0;
}



Album DatabaseAccess::openAlbum(const std::string& albumName)
{
    char* errMessage = nullptr;
    Album returnedAlbum;
    std::string sqlStatementS = "SELECT * FROM ALBUMS WHERE NAME = \"" + albumName + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement,callback_album, nullptr, &errMessage);
    returnedAlbum = myAlbum;
    return returnedAlbum;
}



void DatabaseAccess::printAlbums()
{
    char* sqlStatement = "SELECT * FROM ALBUMS;";
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, callback_print, nullptr, &errMessage);
}



void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
    char* errMessage = nullptr;
    int albumid;
    std::string sqlStatementS = "SELECT ID FROM ALBUMS WHERE NAME = \"" + albumName + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    albumid = key;
    std::string sqlStatementS2 = "INSERT INTO PICTURES (NAME, LOCATION, CREATION_DATE, ALBUM_ID) VALUES (\"" + picture.getName() + "\", \"" + picture.getPath() + "\", \"" + picture.getCreationDate() + "\", \"" + to_string(albumid) + "\");";
    sqlStatement = const_cast<char*>(sqlStatementS2.c_str());
    res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}



void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
    char* errMessage = nullptr;
    int albumid;
    std::string sqlStatementS = "SELECT ID FROM ALBUMS WHERE NAME = \"" + albumName + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    albumid = key;
    sqlStatementS = "DELETE FROM PICTURES WHERE ALBUM_ID = \"" + to_string(albumid) + "\" AND NAME = \"" + pictureName + "\";";
    sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}



void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
    //TAKE ID FROM ALBUM, THEN USE IT TO FIND PICTURE ID, THEN CREATE NEW ROW IN TABLE TAGS WITH PICID AND USERID
    char* errMessage = nullptr;
    int albumid, picture_id;
    std::string sqlStatementS = "SELECT ID FROM ALBUMS WHERE NAME = \"" + albumName + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    albumid = key;
    sqlStatementS = "SELECT ID FROM PICTURES WHERE ALBUM_ID = \"" + to_string(albumid) + "\" AND NAME = \"" + pictureName + "\";";
    sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    picture_id = key;
    std::string sqlStatementS2 = "INSERT INTO TAGS (PICTURE_ID, USER_ID) VALUES (\"" + to_string(picture_id) + "\", \"" + to_string(userId) + "\");";
    sqlStatement = const_cast<char*>(sqlStatementS2.c_str());
    res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}



void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
    //TAKE ID FROM ALBUM, THEN USE IT TO FIND PICTURE ID, THEN CREATE NEW ROW IN TABLE TAGS WITH PICID AND USERID
    char* errMessage = nullptr;
    int albumid, picture_id;
    std::string sqlStatementS = "SELECT ID FROM ALBUMS WHERE NAME = \"" + albumName + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    albumid = key;
    sqlStatementS = "SELECT ID FROM PICTURES WHERE ALBUM_ID = \"" + to_string(albumid) + "\" AND NAME = \"" + pictureName + "\";";
    sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    res = sqlite3_exec(db, sqlStatement, callback_id, &picture_id, &errMessage);
    picture_id = key;
    std::string sqlStatementS2 = "DELETE FROM TAGS WHERE PICTURE_ID = \"" + to_string(picture_id) + "\" AND USER_ID = \"" + to_string(userId) + "\"; ";
    sqlStatement = const_cast<char*>(sqlStatementS2.c_str());
    res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}



void DatabaseAccess::printUsers()
{
    char* sqlStatement = "SELECT * FROM USERS;";
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, callback_print, nullptr, &errMessage);
}



void DatabaseAccess::createUser(User& user)
{
    std::string sqlStatementS = "INSERT INTO USERS (NAME) VALUES (\"" + user.getName() + "\");";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}

void DatabaseAccess::deleteUser(const User& user)
{
    char* errMessage = nullptr;
    std::string sqlStatementS = "DELETE FROM USERS WHERE NAME = \"" + user.getName() + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, &errMessage);
}
bool DatabaseAccess::doesUserExists(int userId)
{
    char* errMessage = nullptr;
    int doesExist = 0;
    std::string sqlStatementS = "SELECT EXISTS(SELECT 1 FROM USERS WHERE ID = \"" + to_string(userId) + "\");";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, &doesExist, &errMessage);
    doesExist = key;
    if (doesExist != 1)
    {
        return false;
    }
    else
        return true;
}
int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
    std::string sqlStatementS = "SELECT ID FROM USERS WHERE NAME = \"" + user.getName() + "\";";
    char* errMessage = nullptr;
    int userid;
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    errMessage = nullptr;
    userid = key;
    int amountOfAlbums = 0;
    sqlStatementS = "SELECT COUNT(ID) FROM ALBUMS WHERE USER_ID = \"" + to_string(userid) + "\");";
    sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    userid = amountOfAlbums;
    return amountOfAlbums;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
    std::string sqlStatementS = "SELECT ID FROM USERS WHERE NAME = \"" + user.getName() + "\";";
    char* errMessage = nullptr;
    int userid;
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    int res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    errMessage = nullptr;
    userid = key;
    int amountOfTags = 0;
    sqlStatementS = "SELECT COUNT(ID) FROM TAGS WHERE USER_ID = \"" + to_string(userid) + "\");";
    sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    res = sqlite3_exec(db, sqlStatement, callback_id, nullptr, &errMessage);
    amountOfTags = key;
    return amountOfTags;
}
float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
    return static_cast<float>(countTagsOfUser(user)) / countAlbumsTaggedOfUser(user);
}
void DatabaseAccess::closeAlbum(Album& album)
{
    /*The close album function and the delete album function go to the database
    and delete the album forever compared to
    the close album that was deleted from the ram but remains in the database
    */
}
int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
    return 0;
}
int callback_albumList(void* data, int argc, char** argv, char** azColName)
{
    albumsList.push_back(Album(atoi(argv[3]), argv[1], argv[2]));
    return 0;
}
const std::list<Album> DatabaseAccess::getAlbums()
{
    char* sqlStatement = "SELECT * FROM ALBUMS;";
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, callback_albumList, nullptr, &errMessage);
    return albumsList;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
    std::string sqlStatementS = "SELECT * FROM ALBUMS WHERE USER_ID = \"" + to_string(user.getId()) + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, callback_albumList, nullptr, &errMessage);
    return albumsList;
}
User DatabaseAccess::getUser(int userId)
{
    std::string sqlStatementS = "SELECT NAME FROM USERS WHERE ID = \"" + to_string(userId) + "\";";
    char* sqlStatement = const_cast<char*>(sqlStatementS.c_str());
    char* errMessage = nullptr;
    int res = sqlite3_exec(db, sqlStatement, callback_user, nullptr, &errMessage);
    myuser.setId(userId);
    return myuser;
}

User DatabaseAccess::getTopTaggedUser()
{
    User* duumy = nullptr;
    return *duumy;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
    Picture* duumy = nullptr;
    return *duumy;
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
    return std::list<Picture>();
}

void DatabaseAccess::clear()
{
}