#include <dirent.h>
#include <stdio.h>
#include <errno.h>

void list_directory(char* name)
{
    DIR* dirptr=opendir(name);
    struct dirent *dit;
    while((dit = readdir(dirptr)) != NULL){
    
    switch(dit->d_type){
        case DT_REG:
            printf("name:%s ", dit->d_name);
            printf("type: DT_REG\n");
        break;
        case DT_DIR:
            printf("name:%s ", dit->d_name);
            printf("type: DT_DIR\n");
        break;
    }
    }
    
    if(dirptr){
        closedir(dirptr);
        }
}

int main()
{
//makrók kiíratása
printf("blokk dev:%d, charakter:%d, directory:%d,regular file:%d\n", DT_BLK, DT_CHR, DT_DIR, DT_REG);

    //megadom a listázandó könyvzár nevét
    list_directory("test");
    
    return 0;
}


