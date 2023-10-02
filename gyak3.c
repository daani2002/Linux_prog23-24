#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

//törli a paraméterként megadott könyvtár tartalmát
void list_directory(char* name)
{
    DIR* dirptr=opendir(name);
    //hiba megnyitáskor (pl. nincs ilyen directory)
    if(dirptr==NULL){
        printf("Nem sikerült megnyitni a könyvtárat!\n");
        return ;
        }
    struct dirent *dit;
    char path[PATH_MAX];
    int status;
    
    //kiolvason a fájlfolyam következő elemét
    while((dit = readdir(dirptr)) != NULL){
        //elérési út szerkesztése: fájlnév/könyvtár
        snprintf(path, sizeof(path), "%s/%s", name, dit->d_name);
        //külön kezelem a fájlt és könyvtárat
        switch(dit->d_type){
            //regular file, törlöm
            case DT_REG:
                printf("name:%s/%s\t", name, dit->d_name);
                printf("type: DT_REG\n");
                status=remove(path);
                if(status==0)
                    printf("fájl törölve\n");
                else
                    printf("nem sikerült törölni!");
            break;
            //könytárba belépek, törlöm a fájlokat rekurzív hívással
            case DT_DIR:
            // . .. könyvtárakat kihagyom
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
        
    //bezárás után törlöm az aktuális könytárat is
    status=remove(name);
    if(status==0)
        printf("mappa törölve\n");
    else
        printf("nem sikerült törölni!");
}

int main(int argc, char* argv[])
{
    //nincs megadva parméter
    if(argc < 2)
        printf("Adja meg a törlendő könyvtár nevét\n");
    else
    //átadom a listázandó könyvtár nevét
    list_directory(argv[1]);
    
    return 0;
}


