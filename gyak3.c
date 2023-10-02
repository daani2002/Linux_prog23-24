#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

void list_directory(char* name)
{
    DIR* dirptr=opendir(name);
    struct dirent *dit;
    char path[PATH_MAX];
    int status;
    
    while((dit = readdir(dirptr)) != NULL){
        snprintf(path, sizeof(path), "%s/%s", name, dit->d_name);
        switch(dit->d_type){
            case DT_REG:
                printf("name:%s/%s\t", name, dit->d_name);
                printf("type: DT_REG\n");
                status=remove(path);
                if(status==0)
                    printf("fájl törölve\n");
                else
                    printf("nem sikerült törölni!");
            break;
            case DT_DIR:
            if(strcmp(dit->d_name,".") && strcmp(dit->d_name,".."))
            {
                printf("name:%s/%s\t", name, dit->d_name);
                printf("type: DT_DIR\n");
                //rekurzió
                list_directory(path);
            }
            break;
        }
    }
    
    if(dirptr){
        closedir(dirptr);
        }
        
    status=remove(name);
    if(status==0)
        printf("mappa törölve\n");
    else
        printf("nem sikerült törölni!");
}

int main()
{

    //megadom a listázandó könyvtár nevét
    list_directory("test");
    
    return 0;
}


