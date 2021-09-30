// Library Includes
#include <SD.h>
#include "Scene.h"

typedef char FILENAME[8+1+3+1];

// Scene files list
FILENAME *sceneNames = NULL;
uint16_t cntScenes = 0;

// SD Card
//SdFat* SD = NULL;
//SD sd;

const int ledPin = 13;

//----------------
// Function: setup
//----------------
void setup()
{  
  pinMode(ledPin, OUTPUT);
  
  // Serial debug
  Serial.begin(115200);
  while(!Serial);

  // Randomize the generator
  randomSeed(analogRead(32));

  // Init the SD Card
  if(InitSD())
  {  
    // Init the scene file name list
    InitScenes();
    Serial.println(cntScenes);
  }
}

//---------------
// Function: loop
//---------------
void loop()
{
  if((millis() / 1000) % 2 == 0)
  {
    // On
    digitalWrite(ledPin, HIGH);
  }
  else
  {
    // Off
    digitalWrite(ledPin, LOW);
  }
  
  doClock();
}

//------------------
// Function: doClock
//------------------
void doClock()
{
  static FsFile fileScene ;
  static Scene scene;

  if(cntScenes > 0 && !fileScene.isOpen())
  {
    char pathScene[255 + 1];

    // Not debug, so play random scene
    sprintf(pathScene, "/Scenes/%s", sceneNames[random(cntScenes)]);

    // Open the scene file
    fileScene = SD.sdfs.open(pathScene, O_RDONLY);
    if(fileScene.isOpen())
    {
      // Creste the scene object from the scene file
      if(!scene.Create(fileScene))
      {
        // SD Card not inserted
        fileScene.close();
        cntScenes = 0;
      }
    }
    else
    {
      // SD Card not inserted
      cntScenes = 0;
    }
  }

  // At the end of the scene?
  if(!scene.Eof())
  {
    // First frame or next frame
    scene.NextFrame(fileScene);
  }
  else
  {
    // Finished the scene close it
    fileScene.close();
    //ReInitSD();
  }
}

//-----------------
// Function: InitSD
//-----------------
bool InitSD()
{
    return SD.sdfs.begin(SdioConfig(DMA_SDIO)) && SD.exists("/Scenes");
}

//bool InitSD()
//{
//  if(SD == NULL)
//  {
//    SD = new SdFat;

    // Connect to SD Card
    // Return whether we can see the Scenes directory
//    return SD->begin() && SD->exists("/Scenes");
//  }  
//  else
//  {
//    return SD->exists("/Scenes");
//  }
//}

//-------------------
// Function: ReInitSD
//-------------------
//bool ReInitSD()
//{
//  if(SD != NULL)
//  {
//    delete SD;
//    SD = NULL;
//  }

//  return InitSD();
//}

//---------------------
// Function: InitScenes
//---------------------
void InitScenes()
{
  FsFile dirScenes;
  FsFile file ;

  // Scene directory exists?
  dirScenes = SD.sdfs.open("/Scenes", O_RDONLY);
  if(dirScenes.isOpen())
  {
    // Previous list exists?
    if(sceneNames != NULL)
    {
      // Free it
      free(sceneNames);

      cntScenes = 0;
      sceneNames = NULL;
    }

    // Loop on each scene file
    while(file.openNext(&dirScenes, O_RDONLY))
    {
        FILENAME filename ;

        if(file.getName(filename, sizeof(filename)))
        {
          // Don't want to store the branding scene
          if(strcmp(filename, "BRAND.SCN") != 0)
          {
            if(sceneNames == NULL)
            {
              // First time, malloc
              cntScenes = 1;        
              sceneNames = (FILENAME *)malloc(sizeof(FILENAME));
            }
            else
            {
              // Subsequent time, realloc
              cntScenes++;
              sceneNames = (FILENAME *)realloc(sceneNames, sizeof(FILENAME) * (cntScenes));
            }
  
            // Copy the file name into the ever expanding array
            strcpy(sceneNames[cntScenes - 1], filename);
        }
      }

      file.close();
    }

    // Sort the list alphabetically
    if(sceneNames != NULL && cntScenes > 0)
    {
      // Sort the list of filenames alphabetically
      qsort(sceneNames, cntScenes, sizeof(sceneNames[0]), (__compar_fn_t)strcmp);
    }

    // Close the directory
    dirScenes.close();
  }
}
