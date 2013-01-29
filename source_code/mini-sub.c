/* Mini-Sub Version 1.0
* Created Jan 2013
* 
* Created by Andrew Hazelden
* Email andrew@andrewhazelden.com
*
* Required Graphics Libraries:
* SDL "Simple DirectMedia Layer"
* http://www.libsdl.org/
*  
* SDL_TTF
* http://www.libsdl.org/projects/SDL_ttf/
*  
* SDL_gfx
* http://www.ferzkopp.net/joomla/content/view/19/14/  
* 
* SDL_mixer
* http://www.libsdl.org/projects/SDL_mixer/
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>  /* For time(), used with srand(). */

#ifdef _WIN32
#include <SDL/SDL.h>      /* Adds graphics support */
#include <SDL/SDL_ttf.h>            /* Adds font support */
#include <SDL/SDL_rotozoom.h>       /* SDL_gfx Rotozoom  */
#include <SDL/SDL_gfxPrimitives.h>	/* SDL_gfx Primitives */
#include <SDL/SDL_framerate.h>	/* SDL_gfx Framerate Manager */
#include "SDL_mixer.h"
#else
#include <SDL.h>
#include <SDL_ttf.h>            /* Adds font support */
#include <SDL_rotozoom.h>       /* SDL_gfx Rotozoom  */
#include <SDL_gfxPrimitives.h>	/* SDL_gfx Primitives */
#include <SDL_framerate.h>	/* SDL_gfx Framerate Manager */
#include "SDL_mixer.h" 
#endif


#include "mini-sub.h"


/* ------------------------------- */
/* shared mini-sub setup           */
/* ------------------------------- */

/* Mini-Sub player dimensions  */
#define PLAYER_HEIGHT 47
#define PLAYER_WIDTH 63

/* Status bar height     */
#define STATUS_TEXT_CELL_HEIGHT 25

int torpedoButton=0;

int muteButton=0;


void UART1_Write_Line(char *uart_text);
void UART1_Write_Variable(int var);
void UART1_Write_Long_Variable(long var);

void UART1_Write_Label_Var(char *uart_text, int var );
void UART1_Write_Label_Long_Var(char *uart_text, long var);
void UART1_Write_Label_Float_Var(char *uart_text, float var);
int TFT_Write_Text(char *textstring, int x_pos, int y_pos);
int TFT_Write_Big_Text(char *textstring,int x_pos, int y_pos);

void SwitchFullscreen();
void ToggleFullscreen();

void Play_WAV();
void WAV_Start();

//define the startup sound volume
void Startup_Volume( char vol);

extern char sound_level, old_sound_level;
extern void MP3_Set_Volume(char left, char right);
void UpdateVolumeBar(char vol, char old_vol);

//Count how many songs have played
extern int song_count;

//Flag to indicate that the current song is done playing
extern int play_next_song;


/* mini-sub Console size */ 
int screen_width = 800;
int screen_height = 600;
int enable_fullscreen = 0;

/* Console text size in points */
int fontsize = 18;  /* default font size is 16 points for 640x480 */

int character_with = 0;
int character_height = 0;

int big_character_with = 0;
int big_character_height = 0;

int total_text_rows = 0;
int total_text_cols = 0;


/* User input */
int isbutton_down = 0; /* read if the mouse button was pressed */
int mousex = 0; /* read the mousex position */
int mousey = 0; /* read the mousey position */


/* SDL done flag */
int done;

/* Set the default frame rate */
const int FPS = 30;

/* Program uptime counter */
long int start_time = 0;

SDL_Surface *program_icon;



int main (int argc, char *argv[])
{
  int inc=0;

  FPSmanager fps_manager;
   
  start_time = time(NULL);
  
  GetArguments(argc, argv); /* Get the script filename */
  PrintVersion();  /* Print info */
  InitGFX();       /* Setup SDL  */	
  LoadSprites();	 /* Load the BMP images  */
  
	// Set the FramesPerSecond
	SDL_initFramerate( &fps_manager );
	SDL_setFramerate( &fps_manager, FPS );   

  
  WAV_Start();	/* Setup SDL Mixer */
  Play_WAV();		/* Start the background soundtrack */
  UART1_Write_Line("Mini-Sub Game Started"); 

  while(!done){
    //Reset the game
    ResetGame();
    
    start_game_flag = 1;
    
    //Run the game until the player is out of lives
    while(subLives && !done){
      
      //Reset the sprites
      InitSprites();
      
      //Run the current level until the sub hits a mine
      while((mineHit == 0) && !done) {
        
        //Play the background music
        if( (frame_counter % 10) == 0){
          //make sure the soundtrack is still playing every nth frame
          Play_WAV();
        }
        
        
        //Check for user input
        GetInput();
        
        //Draw the graphics
        RenderScreen();
        
        //Store the previous values
        SavePreviousVal();
        
        //Increment the frame counter
        frame_counter++;
        
        //Slow down the graphics update to 30 fps
        SDL_framerateDelay( &fps_manager); // this will delay execution for a while
        //SDL_Delay(33);
        
      } //End of the current sub life
      
      //Reset the mine hit counter
      mineHit = 0;
      
      //Reset the mineExplode counter
      mineExplode = 0;
      
      //Reduce the life counter because a submarine blew up
      subLives--;
    }
    
    //Display the game over title screen
    ShowGameOver();
  }
  
  
  
  /* Release SDL_ttf font files */
  TTF_CloseFont( fnt );
  TTF_CloseFont( fnt_big);
  
  /* Release SDL mixer and the sound files */
  Mix_FreeChunk(sound);
  
  /* Close SDL Mixer */
  Mix_CloseAudio();
  
  /* Release the SDL sprites */
  FreeSprites();
  
  return 0;
}


int GetMouseX(void)
{
  return mousex;
}

int GetMouseY(void)
{
  return mousey;
}

int GetMouseButton(void)
{
  if(isbutton_down)
  {
    printf("you clicked\n");
  }
  return isbutton_down;
}




/* read the argv values */
void GetArguments(int argc, char *argv[])
{
  int scanner = 0;
  
  char option1[128];
  char option2[128];
  int tempwidth, tempheight;
  
  printf("Usage: mini-sub [options]\n");
  printf("Options:\n");
  printf("  --window <width> <height> [-fullscreen]\n");
  printf("    Set the size of the main window\n");
  
  /*printf("Argv contents:\n");*/
  for(scanner=0;scanner<=argc;scanner++)
  {
    printf("[%d]%s\n",scanner, argv[scanner]);
  } /*end for scanner*/
  
  if(argc >=2)
  {
    /* check for the screen size argument */	
    if(argv[1] != NULL)
    {
      strcpy(option1, argv[1]); 
      /*check if the --window option is in the argv*/
      if( strstr(option1, "--window") )
      {
        /* you found the window option - this requires width and height */
        if((argv[2] != NULL) && (argv[3] != NULL))
        { 
          tempwidth = atoi(argv[2]);
          tempheight = atoi(argv[3]);
          printf("Got a --window size of width=%d height=%d\n", tempwidth, tempheight);
          if (tempwidth >= 640) 
          {
            screen_width = tempwidth;
          }
          if(tempheight >= 400)
          {
            screen_height = tempheight;
          }
          
          /* Check for fullscreen option */
          if(argv[4] != NULL)
          {
            strcpy(option2, argv[4]); 
            /*check if the --window option is in the argv*/
            if( strstr(option2, "-fullscreen") )
            {
              enable_fullscreen=1;
              printf("Got a -fullscreen\n");
            }
            else
            {
              enable_fullscreen=0;
            }
            
          }
        }					
      } /*end argv null check*/
    }		
  }
}




/* Game version info */
void PrintVersion(){
  printf("\nWelcome to the Mini-Sub Game V1.0\n");
  printf("--------------------------------------------\n");
  printf("Created by Andrew Hazelden\n");
  
#ifdef _WIN32
  printf("Running on Windows\n");	
#elif __sgi
  printf("Running on IRIX\n");	
#elif __APPLE__
  printf("Running on Mac OS X\n");
#elif __linux__
  printf("Running on Linux\n");
#endif
  
  printf("\n");
}

/* Load the sound system */
void WAV_Start(){
  /* Load the SDL Mixer audio system */
  if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
    printf("Unable to initialize audio: %s\n", Mix_GetError());
    exit(1);
  }
  
  /* Load the WAV soundtrack */
  sound = Mix_LoadWAV("resources/Pavese.wav");
  if(sound == NULL) {
    printf("Unable to load WAV file: %s\n", Mix_GetError());
  }
  
}

/* Play a sound file */
void Play_WAV(){
  /* Loop the sound if it has stopped */
  if(Mix_Playing(channel) == 0){
    /* Play a sound file, and capture the channel on which it is played */
    channel = Mix_PlayChannel(-1, sound, 0);
    if(channel == -1) {
      printf("Unable to play WAV file: %s\n", Mix_GetError());
    }
  }
}

/* Setup SDL */
void InitGFX(){
  //static int flags=0;
  flags=0;

  /* Define the program icon */
#ifdef _WIN32
  program_icon = SDL_LoadBMP("resources/sub-icon-32px.bmp");
  //key the blue background from the player sprite
  SDL_SetColorKey( program_icon, SDL_SRCCOLORKEY, SDL_MapRGB(program_icon->format, 4, 98, 171) );
  
#else
  program_icon = SDL_LoadBMP("resources/sub-icon-64px.bmp");
  //key the blue background from the player sprite
  SDL_SetColorKey( program_icon, SDL_SRCCOLORKEY, SDL_MapRGB(program_icon->format, 4, 98, 171) );
  
#endif
  
  char *msg;
  
  int audio_rate = 22050;
  Uint16 audio_format = AUDIO_S16SYS;
  int audio_channels = 2;
  int audio_buffers = 4096;
  
  /* Initialize SDL */
  if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
  {
    sprintf (msg, "Couldn't initialize SDL: %s\n", SDL_GetError ());
    /*MessageBox (0, msg, "Error", program_icon);*/
    free (msg);
    exit (1);
  }
  atexit (SDL_Quit);
  
  /* Set video mode */
  screen = SDL_SetVideoMode (screen_width, screen_height, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
  
  if (screen == NULL)
  {
    sprintf (msg, "Couldn't set %dx%dx16 video mode: %s\n", screen_width, screen_height, SDL_GetError ());
    printf("%s",msg);
    free (msg);
    exit (2);
  }
  
  flags = screen->flags;
  
  /* Check if the enable fullscreen argument is turned on */
  if(enable_fullscreen){
    SwitchFullscreen();
  }
  
  
  
  SDL_WM_SetIcon(program_icon, NULL);
  SDL_WM_SetCaption ("Mini-Sub Game", "Mini-Sub Game");
  
  /* Create background screen colors*/
  screen_color = SDL_MapRGB(screen->format, 66, 66, 231);
  frame_color = SDL_MapRGB(screen->format, 165, 165, 255);
  debug_color = SDL_MapRGB(screen->format, 126, 126, 126);
  
  
  /* light blue */
  foreground_color.r = 165;
  foreground_color.g = 165;
  foreground_color.b = 255;
  
  /* dark blue */
  background_color.r = 66;
  background_color.g = 66;
  background_color.b = 231;
  
  
  /* Setup a rect for the full screen */
  screen_rect.h = screen->h;
  screen_rect.w = screen->w; 
  
  /* frame border width 36px / height 45px */
  display_area_rect.x = 36;
  display_area_rect.y = 45;
  display_area_rect.w = screen->w-(36*2);
  display_area_rect.h = screen->h-(45*2);
  
  
  
  
  /* Set up the SDL_TTF */
  TTF_Init();
  atexit(TTF_Quit);
  /* TTF_Init() is like SDL_Init(), but with no parameters.  Basically, it initializes
  SDL_TTF.  There's really not much to it.  Remember, when the program ends, we
  have to call TTF_Quit().  atexit(TTF_Quit) ensures that when we call exit(), the
  program calls TTF_Quit() for us. */
  
  fnt = TTF_OpenFont( "resources/Xolonium-Regular.otf", fontsize );
  

  /* Get the size of a character in pixels */
  if(TTF_SizeText(fnt,"B",&character_with,&character_height)) {
    printf("SDL_TTF error: %s", TTF_GetError());
  } else {
    total_text_rows = (display_area_rect.h/character_height)-1;
    total_text_cols = (display_area_rect.w/character_with)-1;
    printf("Current Screen Size: width=%d height=%d\n", screen_width, screen_height);
    printf("Text Settings: %d=points rows=%d columns=%d \n", fontsize, total_text_rows, total_text_cols);
    printf("\n");
  }
  
  fnt_big = TTF_OpenFont( "resources/Xolonium-Regular.otf", fontsize*3 );
  
  
  /* Get the size of a character in pixels */
  if(TTF_SizeText(fnt_big,"B",&big_character_with,&big_character_height)) {
    printf("SDL_TTF error: %s", TTF_GetError());
  } else {
    //total_text_rows = (display_area_rect.h/character_height)-1;
    //total_text_cols = (display_area_rect.w/character_with)-1;
    //printf("Current Screen Size: width=%d height=%d\n", screen_width, screen_height);
    //printf("Text Settings: %d=points rows=%d columns=%d \n", fontsize, total_text_rows, total_text_cols);
    //printf("\n");
  }
  
  
  /*Wait a moment for the screen to switch*/
  if(enable_fullscreen)
  {
    SDL_Flip(screen);
    SDL_Delay(1500);
  }
  
  /*-------------------------------
  * Fill the screen
  * -------------------------------
  */
  
  //Set the ocean background color
  bgColor = SDL_MapRGB(screen->format, 4, 98, 171);
  SDL_FillRect(screen, NULL, bgColor);

  /* Make sure everything is displayed on screen */
  SDL_Flip(screen);
  
}


//Switch the display fullscreen
void SwitchFullscreen(){

  /* Check if the enable fullscreen argument is turned on */
  if(enable_fullscreen){
    screen = SDL_SetVideoMode(0, 0, 0, screen->flags ^ SDL_FULLSCREEN);
    screen_width = screen->w;
    screen_height = screen->h;
    
    if(screen == NULL) 
    {
      screen = SDL_SetVideoMode(0, 0, 0, flags); /* If toggle FullScreen failed, then switch back */
    }
    if(screen == NULL)
    {
      exit(1); /* If you can't switch back for some reason, then fail */
    }
  }
  
}

void ToggleFullscreen(){

  //Enter Fullscreen Mode
  if(enable_fullscreen){
    enable_fullscreen = 0;
    UART1_Write_Line("Entering Fullscreen Mode");
  }
  else if(!enable_fullscreen){
    //Exit Fullscreen Mode
    enable_fullscreen = 1;
    UART1_Write_Line("Exiting Fullscreen Mode");
  }
  
  SwitchFullscreen();
  
  //Set the ocean background color
  bgColor = SDL_MapRGB(screen->format, 4, 98, 171);
  SDL_FillRect(screen, NULL, bgColor);

  /* Make sure everything is displayed on screen */
  SDL_Flip(screen);
  
  SDL_Delay(200);
  
}



/* Load the mini-sub game images into SDL surfaces */
void LoadSprites(){
  
  //------------------
  //Title Image 
  //------------------
  title_bmp = SDL_LoadBMP("resources/title.bmp");
  if(title_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the player sprite
  SDL_SetColorKey( title_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(title_bmp->format, 4, 98, 171) );
  
  title_rect.x = 0;  
  title_rect.y = 0;  
  title_rect.w = title_bmp->w;
  title_rect.h = title_bmp->h;  
  
  //------------------
  // Submarine sprite 
  //------------------
  sub_bmp = SDL_LoadBMP("resources/sub.bmp");
  if(sub_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the player sprite
  SDL_SetColorKey( sub_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(sub_bmp->format, 4, 98, 171) );
  
  sub_rect.x=0;
  sub_rect.y=0;
  sub_rect.w = sub_bmp->w;
  sub_rect.h = sub_bmp->h;
  
  sub_clear_rect.x = sub_rect.x;
  sub_clear_rect.y = sub_rect.y;
  sub_clear_rect.w = sub_rect.w;
  sub_clear_rect.h = sub_rect.h;
  

  //--------------------
  //torpedo icon
  //--------------------
  torpedo_bmp = SDL_LoadBMP("resources/torpedo.bmp");
  if(torpedo_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the player sprite
  SDL_SetColorKey( torpedo_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(torpedo_bmp->format, 4, 98, 171) );  

  torpedo_rect.x=0;
  torpedo_rect.y=0;
  torpedo_rect.w = torpedo_bmp->w;
  torpedo_rect.h = torpedo_bmp->h;
  
  //------------------
  //Sea mine sprites
  //------------------
  
  mine1_bmp = SDL_LoadBMP("resources/mine1.bmp");
  if(mine1_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
   mine2_bmp = SDL_LoadBMP("resources/mine2.bmp");
  if(mine2_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
   mine3_bmp = SDL_LoadBMP("resources/mine3.bmp");
  if(mine3_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
   mine4_bmp = SDL_LoadBMP("resources/mine4.bmp");
  if(mine4_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
   mine5_bmp = SDL_LoadBMP("resources/mine5.bmp");
  if(mine5_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
   mine6_bmp = SDL_LoadBMP("resources/mine6.bmp");
  if(mine6_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  
	//key the blue background from the sprite
	SDL_SetColorKey( mine1_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine1_bmp->format, 4, 98, 171) );  
	SDL_SetColorKey( mine2_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine2_bmp->format, 4, 98, 171) );  
	SDL_SetColorKey( mine3_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine3_bmp->format, 4, 98, 171) );  
	SDL_SetColorKey( mine4_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine4_bmp->format, 4, 98, 171) );  
	SDL_SetColorKey( mine5_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine5_bmp->format, 4, 98, 171) );  
	SDL_SetColorKey( mine6_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(mine6_bmp->format, 4, 98, 171) );  
	
  
  mine_rect.x = 0;
  mine_rect.y = 0;
  mine_rect.w = mine1_bmp->w;
  mine_rect.h = mine1_bmp->h;
  
  
  dark_mine1_bmp = SDL_LoadBMP("resources/dark_mine1.bmp");
  if(dark_mine1_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  dark_mine2_bmp = SDL_LoadBMP("resources/dark_mine2.bmp");
  if(dark_mine2_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  dark_mine3_bmp = SDL_LoadBMP("resources/dark_mine3.bmp");
  if(dark_mine3_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
   
  dark_mine4_bmp = SDL_LoadBMP("resources/dark_mine4.bmp");
  if(dark_mine4_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  dark_mine5_bmp = SDL_LoadBMP("resources/dark_mine5.bmp");
  if(dark_mine5_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  dark_mine6_bmp = SDL_LoadBMP("resources/dark_mine6.bmp");
  if(dark_mine6_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  SDL_SetColorKey( dark_mine1_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine1_bmp->format, 4, 98, 171) );  
  SDL_SetColorKey( dark_mine2_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine2_bmp->format, 4, 98, 171) );  
  SDL_SetColorKey( dark_mine3_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine3_bmp->format, 4, 98, 171) );  
  SDL_SetColorKey( dark_mine4_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine4_bmp->format, 4, 98, 171) );  
  SDL_SetColorKey( dark_mine5_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine5_bmp->format, 4, 98, 171) );  
  SDL_SetColorKey( dark_mine6_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(dark_mine6_bmp->format, 4, 98, 171) );  
  



  //------------------
  //Fireball sprites
  //------------------
  
  fireball_bmp = SDL_LoadBMP("resources/fireball.bmp");
  if(fireball_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  SDL_SetColorKey( fireball_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(fireball_bmp->format, 4, 98, 171) );  
  
  fireball_rect.x = 0;
  fireball_rect.y = 0;
  fireball_rect.w = fireball_bmp->w;
  fireball_rect.h = fireball_bmp->h;   


  //------------------
  //Explode sprites
  //------------------
  
  explode_bmp = SDL_LoadBMP("resources/explode.bmp");
  if(explode_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  SDL_SetColorKey( explode_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(explode_bmp->format, 4, 98, 171) );  
  
  explode_rect.x = 0;
  explode_rect.y = 0;
  explode_rect.w = explode_bmp->w;
  explode_rect.h = explode_bmp->h;   


  //------------------
  //Status bar sprites
  //------------------
  
  score_clear_rect.x = 0;
  score_clear_rect.y = 0;
  score_clear_rect.w = screen_width;
  score_clear_rect.h = STATUS_TEXT_HEIGHT;
  
  //------------------
  //Audio ON icon
  //------------------
  sound_bmp = SDL_LoadBMP("resources/sound.bmp");
  if(sound_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  //SDL_SetColorKey( sound_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(sound_bmp->format, 4, 98, 171) );  
  
  //Position the sound status icon at 25% of the screen width
  //sound_rect.x = 80;
  sound_rect.x = (screen_width/4);
  sound_rect.y = 2;
  sound_rect.w = sound_bmp->w;
  sound_rect.h = sound_bmp->h;
  
  //------------------
  //Audio OFF icon
  //------------------
  soundMute_bmp = SDL_LoadBMP("resources/soundMute.bmp");
  if(soundMute_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  //SDL_SetColorKey( soundMute_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(soundMute_bmp->format, 4, 98, 171) );  
  
  //--------------------
  //Torpedo status icon
  //--------------------
  torpedo_status_bmp = SDL_LoadBMP("resources/torpedo_status.bmp");
  if(torpedo_status_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  //SDL_SetColorKey( torpedo_status_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(torpedo_status_bmp->format, 4, 98, 171) );  
  
  torpedo_status_rect.x = 260;
  torpedo_status_rect.y = 2;
  torpedo_status_rect.w = torpedo_status_bmp->w;
  torpedo_status_rect.h = torpedo_status_bmp->h;
  
  //--------------------
  //lives status icon
  //--------------------
  lives_bmp = SDL_LoadBMP("resources/lives.bmp");
  if(lives_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  //SDL_SetColorKey( lives_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(lives_bmp->format, 4, 98, 171) );  
  
  lives_rect.x = 5;
  lives_rect.y = 2;
  lives_rect.w = lives_bmp->w;
  lives_rect.h = lives_bmp->h;
  
  //--------------------
  //heart pack icon
  //--------------------
  heart_pack_bmp = SDL_LoadBMP("resources/heart_pack.bmp");
  if(heart_pack_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  SDL_SetColorKey( heart_pack_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(heart_pack_bmp->format, 4, 98, 171) );  
  
  bonus_block_rect.x = 5;
  bonus_block_rect.y = 2;
  bonus_block_rect.w = heart_pack_bmp->w;
  bonus_block_rect.h = heart_pack_bmp->h;
  
  //--------------------
  //torpedo pack icon
  //--------------------
  torpedo_pack_bmp = SDL_LoadBMP("resources/torpedo_pack.bmp");
  if(torpedo_pack_bmp == NULL)
  fprintf(stderr, "Unable to load image: %s\n", SDL_GetError());
  
  //key the blue background from the sprite
  SDL_SetColorKey( torpedo_pack_bmp, SDL_SRCCOLORKEY, SDL_MapRGB(torpedo_pack_bmp->format, 4, 98, 171) );  

  
}


// Free the mini-sub images
void FreeSprites(){
  SDL_FreeSurface(title_bmp);
  SDL_FreeSurface(sub_bmp);
  SDL_FreeSurface(mine1_bmp);
  SDL_FreeSurface(dark_mine1_bmp);
  SDL_FreeSurface(sound_bmp);
  SDL_FreeSurface(soundMute_bmp);
  SDL_FreeSurface(torpedo_status_bmp);
  SDL_FreeSurface(lives_bmp);
}


void ShowNextLevel(){
  int loop = 0;

  //Check if the user is on the next level
  if (prev_level != level){

    //Write out the current level
    UART1_Write_Line("Next Level: ");
    UART1_Write_Variable(level);

    //Write out the current score
    UART1_Write_Line("Score: ");
    UART1_Write_Variable(score);

    //Write out the current mine speed
    UART1_Write_Line("Mine Speed: ");
    UART1_Write_Variable(mine_speed);

    //Write out the current bonus factor
    UART1_Write_Line("Bonus distance factor: ");
    UART1_Write_Variable(bonus_factor);
    

    //Fill the screen with the blue sea color
    SDL_FillRect(screen, NULL, bgColor);

    //Display the game title
    //TFT_Image(35, (screen_height/2)-47, title_bmp, 1);
    
    //Convert the level from an int to string
    IntToStr(level, level_text);
    strcpy(level_display_text, "Level: ");
    strcat(level_display_text, level_text);

    //Write the next level
    //TFT_Write_Text(level_display_text, 140, 150);
    PrintBigCenteredTextLine(level_display_text, 150);
    
    //Write "You Earned an Extra Life!"
    //TFT_Write_Text("You  Earned  an  Extra  Life!", screen_width/2, 180);
    PrintCenteredTextLine("You  Earned  an  Extra  Life!", 220);
    
    SDL_Flip(screen);
    SDL_Delay(2000);
    
    //Fill the screen with the blue sea color
    SDL_FillRect(screen, NULL, bgColor);
    
    
    //Reset the sprite positions for the next level
    InitSprites();
  }
}



//Display the game over title
void ShowGameOver(){
  int loop = 0;

  UART1_Write_Line("Game Over");
  
  //Fill the screen with the blue sea color
  SDL_FillRect(screen, NULL, bgColor);
  
  //Display the game over title
  //TFT_Image(0, (screen_height/2)-47, GameOver_bmp, 1);
  PrintBigCenteredTextLine("Game Over", (screen_height/2)-72);
  

  //Check if you set a high score:
  if(score > high_score){
    //You have a high score
    high_score = score;
    PrintCenteredTextLine("You  set  a  High  Score!", 140);
  }
  else{
    //Show the previous high score
    
    //Convert the high_score from an int to string
    IntToStr(high_score, score_text);
    strcpy(score_display_text, "High  Score: ");
    strcat(score_display_text, score_text);
    strcat(score_display_text, "\0");

    //Write the high score
    PrintCenteredTextLine(score_display_text, 140);
  }
  
  //Convert the score from an int to string
  IntToStr(score, score_text);
  strcpy(score_display_text, "Your  Score: ");
  strcat(score_display_text, score_text);
  strcat(score_display_text, "\0");

  //Write the score
  PrintCenteredTextLine(score_display_text, 170);
  
  SDL_Flip(screen);	
  SDL_Delay(2000);

  //Fill the screen with the blue sea color
  //SDL_FillRect(screen, NULL, bgColor);
}





int PrintBigCenteredTextLine(char *textstring, int y_pos){
  SDL_Color text_color;
  //text_color.r = foreground_color.r;
  //text_color.g = foreground_color.g;
  //text_color.b = foreground_color.b;
  
  //yellow text
  text_color.r = 239;
  text_color.g = 189;
  text_color.b = 0;
  
  
  SDL_Surface *textSurface = TTF_RenderText_Blended(fnt_big, textstring, text_color);
  
  SDL_Rect textDestRect;
  
  if (textSurface) {
    /* now center it horizontally and position it just below the logo */
    textDestRect.x = (screen->w - textSurface->w) / 2;
    textDestRect.y = y_pos;
    textDestRect.w = textSurface->w;
    textDestRect.h = textSurface->h;
    SDL_BlitSurface(textSurface, NULL, screen, &textDestRect);
    SDL_FreeSurface(textSurface);
  }
  //SDL_Flip(screen);	
  return textDestRect.w;
}


int PrintCenteredTextLine(char *textstring, int y_pos){
  SDL_Color text_color;
  //text_color.r = foreground_color.r;
  //text_color.g = foreground_color.g;
  //text_color.b = foreground_color.b;
  
  //yellow text
  text_color.r = 239;
  text_color.g = 189;
  text_color.b = 0;
  
  
  SDL_Surface *textSurface = TTF_RenderText_Blended(fnt, textstring, text_color);
  
  SDL_Rect textDestRect;
  
  if (textSurface) {
    /* now center it horizontally and position it just below the logo */
    textDestRect.x = (screen->w - textSurface->w) / 2;
    textDestRect.y = y_pos;
    textDestRect.w = textSurface->w;
    textDestRect.h = textSurface->h;
    SDL_BlitSurface(textSurface, NULL, screen, &textDestRect);
    SDL_FreeSurface(textSurface);
  }
  //SDL_Flip(screen);
  return textDestRect.w;
}


//Check for user input
void GetInput(){
  SDL_Event event;
  int mine_inc = 0;
  int edge_border = 3;
  
  //-------------------
  //Check for events
  //-------------------
  
  
  while (SDL_PollEvent (&event))
  {
    switch (event.type)
    {
    case SDL_KEYDOWN:
      switch( event.key.keysym.sym ){
      case SDLK_LEFT:      
        leftButton=1; //Left
        break;
      case SDLK_RIGHT:
        rightButton=1; //Right
        break;
      case SDLK_UP:
        upButton=1; //Up
        break;
      case SDLK_DOWN:
        downButton=1; //Down
        break;
      case SDLK_SPACE:
        torpedoButton=1;  //Fire button
        break;
      default:
        break;
      }
      break;
    case SDL_KEYUP:
      switch( event.key.keysym.sym ){
      case SDLK_LEFT:
        leftButton=0; //Left
        break;
      case SDLK_RIGHT:
        rightButton=0; //Right
        break;
      case SDLK_UP:
        upButton=0; //Up
        break;
      case SDLK_DOWN:
        downButton=0; //Down
        break;
      case SDLK_ESCAPE:
        done = 1;  /* Quit when a key is pressed */
        printf("Escape key pressed.\n");
        break;
      case SDLK_m:
        ToggleMute();   //Switch Audio ON/OFF
        break;
      case SDLK_SPACE:
        torpedoButton=0;  //Fire button
        break;
      case SDLK_TAB:
      	ToggleFullscreen();
      	break;
      default:
        break;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      torpedoButton=1;  //Fire button
      
      isbutton_down=1;
      //get the mouse position when the button is pressed
      SDL_GetMouseState(&mousex, &mousey);
      break;
    case SDL_MOUSEBUTTONUP:
      torpedoButton=0;  //Fire button
      isbutton_down=0;
      break;	
    case SDL_QUIT:
      done = 1;
      break;
    default:
      break;
    }
  } 
  
  
  
  if(upButton) {
    sub.y -= SUB_Y_SPEED;
    //UART1_Write_Line("Up");
  }
  if(downButton){
    sub.y += SUB_Y_SPEED;
    //UART1_Write_Line("Down");
  }
  if(leftButton) {
    sub.x -= SUB_SPEED;
    //UART1_Write_Line("Left");
  }
  if(rightButton){
    sub.x += SUB_SPEED;
    //UART1_Write_Line("Right");
  }
  if(!upButton && !downButton && !leftButton && !rightButton){
    
    //Only animate the sub back to the resting position every nth frame
    //if( (frame_counter % 2) == 0){
      //Slowly animate the submarine back to its' default x axis resting position
      if(sub.x > DEFAULT_SUB_X_POS){
        sub.x -= 3;
      }
    
    //}
    
  }
  

  //Check that the sub.y position is below the status text area
  if( sub.y <= STATUS_TEXT_CELL_HEIGHT ){
    sub.y = STATUS_TEXT_CELL_HEIGHT;
  }

  //Check that the sub.x position is greater than 0
  if( sub.x <= 0 ){
    sub.x = 0;
  }

  //Check that the sub.y position is is still onscreen
  if( sub.y >= (screen_height-PLAYER_HEIGHT)){
    sub.y = screen_height-PLAYER_HEIGHT;
  }

  //Check that the sub.x position is still onscreen
  if( sub.x >= (screen_width-PLAYER_WIDTH)){
    sub.x = screen_width-PLAYER_WIDTH;
  }

  //Scan the sea mine array
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){

    //Check if the mine.x value is less 0 the sea mine has gone offscreen
    if( mine[mine_inc].x <= edge_border){

      //Draw the new mine position staring on the right edge of the screen
      ResetSeaMine(mine_inc);

      //mine_clear_rect.x = mine[mine_inc].prev_x-1;
      //mine_clear_rect.y = mine[mine_inc].prev_y-1;
      //mine_clear_rect.w = mine[mine_inc].width + 1;
      //mine_clear_rect.h = mine[mine_inc].height + 1;
      
      //SDL_FillRect(screen, &mine_clear_rect, bgColor);

      //Increase the score after the mine goes off screen
      score += 25;
    }
    else{
      //Animate the sea mine to the left
      mine[mine_inc].x -= mine_speed;
    }

  }
  
  //Check if the bonus block x position is still onscreen
  if(bonus_block.x<=edge_border){
    //bonus_block_clear_rect.x = bonus_block.prev_x-1;
    //bonus_block_clear_rect.y = bonus_block.prev_y-1;
    //bonus_block_clear_rect.w = bonus_block.width + 1;
    //bonus_block_clear_rect.h = bonus_block.height + 1;

    //SDL_FillRect(screen, &bonus_block_clear_rect, bgColor);
    
    //reset the bonus block position
    ResetBonusBlock();
  }
  
  
  //animate the bonus block across the screen
  bonus_block.x -= mine_speed;



  if(torpedoButton){
    //UART1_Write_Line("Fire");

    if(torpedo_count>=1) {
      //Check for first fired button state and reset the torpedo position
      if( fire_torpedo_flag == 0 ) {
        //The torpedo inherits the starting position of the sub
        torpedo.x = sub.x + PLAYER_WIDTH + 1;

        //Set torpedo to launch from the center of the sub
        torpedo.y = sub.y + (PLAYER_HEIGHT/2);

        //The torpedoe visible
        torpedo.visible = 1;

        //Set the flag that the torpedo has been launched
        fire_torpedo_flag = 1;

        //Reduce the torpedo count by 1
        torpedo_count--;
        
      }
    }

  }


  //Check if there are any torpedoes remaining
  if( fire_torpedo_flag == 1 ){
    torpedo.x += TORPEDO_SPEED;

    //Check if the torpedo went off the screen
    if( torpedo.x > (screen_width+TORPEDO_SPEED+1) ){
      //Hide the torpedo
      torpedo.visible = 0;
      //Reset fire_torpedo flag
      fire_torpedo_flag = 0;
    }

  }
  
}



void ToggleMute(){
  //Turn on sound
  if(muteSound){
    muteSound = 0;
    UART1_Write_Line("Sound On");
    
  }
  else if(!muteSound){
    //Mute the sound
    muteSound = 1;
    UART1_Write_Line("Muting Sound");
  }
  
  SDL_PauseAudio(muteSound);
  SDL_Delay(50);

}


void ResetGame(){

  //Fill the screen with the blue sea color
  SDL_FillRect(screen, NULL, bgColor);
  SDL_Flip(screen);

  //Reset the current level number
  level = 1;
  prev_level = 1;

  //Reset the score
  score = 0;

  //High levels test
  //score = 10025;
  //level = 7;
  //prev_level = 6;
  
  
  //reset the game title text position
  title_origin_x = (screen_width/2)-250;


  //Reset the starting bonus frequency factor
  bonus_factor = 1;

  //Reset the horizontal mine speed
  mine_speed = 6;

  //How many torpedos the sub starts with
  torpedo_count = NUMBER_OF_TORPEDOES;

  //Reset the number of submarine lives
  subLives = NUMBER_OF_SUBMARINES;

  //Reset the number of bonuses displayed counter
  bonus_counter = 0;

  //The rate from 1 to 5 of a grey seamine occuring
  grey_mine_rate = 5;

  //Reset the sprites
  InitSprites();
}


void InitSprites(){
  int mine_inc = 0;
  
  //Reset the number of torpedos the sub has
  torpedo_count = NUMBER_OF_TORPEDOES;
  
  //Reset torpedo fired flag
  fire_torpedo_flag = 0;
  
  
  //Sub sprite
  sub.x = 28;
  sub.y = 128;
  sub.prev_x = sub.x;
  sub.prev_y = sub.y;
  
  
  //Sub explosion Fireball sprite
  fireball.x = 0;
  fireball.y = 0;
  fireball.prev_x = 0;
  fireball.prev_y = 0;
  fireball.width = 130;
  fireball.height = 134;
  
  
  //Sea mine explosion
  explode.x = 0;
  explode.y = 0;
  explode.prev_x = 0;
  explode.prev_y = 0;
  explode.width = 70;
  explode.height = 60;
  
  //torpedo sprite
  torpedo.x = 0;
  torpedo.y = 0;
  torpedo.prev_x = 0;
  torpedo.prev_y = 0;
  torpedo.visible = 1;
  torpedo.width = 35;
  torpedo.height = 14;
  
  //Mine sprites starting position
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){
    //mine[mine_inc].x = (mine_inc * 90) + 200;
    mine[mine_inc].x = (mine_inc * 90) + screen_width;
    mine[mine_inc].y = RandomHeightY();

    mine[mine_inc].prev_x = mine[mine_inc].x ;
    mine[mine_inc].prev_y = mine[mine_inc].y ;
    mine[mine_inc].width = 45;
    mine[mine_inc].height = 45;

    //Picks the seamine type - either red or grey
    mine[mine_inc].type = RandomSeaMineType();
    
    //Sets the seamine to be a standard red version
    //mine[mine_inc].type = 0;
    
    //Reset the mine hits counter
    mine[mine_inc].hits = 0;
  }
  
  
  //Torpedo bonus block sprite
  //Start the block on the 3rd screen
  bonus_block.x = screen_width*BONUS_SCREEN_SPACING;
  bonus_block.y = RandomHeightY();  //generate random block Y position
  bonus_block.prev_x = 0;
  bonus_block.prev_y = 0;
  bonus_block.visible = 1;
  bonus_block.width = 45;
  bonus_block.height = 45;
  
  //set the starting bonus to be a torpedo pack
  bonus_type = 1;
  
  //Fill the screen with the blue sea color
  //SDL_FillRect(screen, NULL, bgColor);
  //SDL_Flip(screen);
  
}



//Generate a random Y height value
int RandomHeightY(){
  int number_of_mine_rows = 10;  //6 rows for a 320x240 display,  10 for an 800x600 display
  return (rand() % number_of_mine_rows) * ((screen_height-STATUS_TEXT_CELL_HEIGHT)/number_of_mine_rows) + STATUS_TEXT_CELL_HEIGHT;
}

int RandomSeaMineType(){
  int mine_type = 0;

  //The grey_mine_rate goes from 1 to 5 of a grey sea mine occuring

  //calculate the random chance of a grey sea mine happening
  mine_type = rand() % grey_mine_rate;

  //Set the percent of the seamines to be grey mines
  if(mine_type == 0){
    return 1;
  }
  else{
    //The rest of the mines are standard red mines
    return 0;
  }
  
  return 0;
}

void ResetBonusBlock(){
  //Reset the position of the bonus block

  //Push the bonus block 4 screens to the right
  bonus_block.x = screen_width*(BONUS_SCREEN_SPACING*bonus_factor);
  
  //Randomize the block height
  bonus_block.y = RandomHeightY();
  
  //Add 1 to the bonus displayed counter
  bonus_counter += 1;
  
  //If the bonus is rare then make it an extra life
  
  //This random function gives a one in 3 chance for an extra life
  if( (rand() % 3) == 0){
    //Set the bonus to be a heart
    bonus_type = 2;
  }
  else {
    //Set the bonus to be a torpedo pack
    bonus_type = 1;
  }
}


void ResetSeaMine(int mineNumber){
  //Draw the new mine position staring on the right edge of the screen
  //Offset the mine Y position below the text area
  mine[mineNumber].x += screen_width;
  mine[mineNumber].y = RandomHeightY();
  
  //Picks the seamine type - either red or grey
  mine[mineNumber].type = RandomSeaMineType();
  
  //Reset the mine torpedo hit counter
  mine[mineNumber].hits = 0;
}


//Draw the graphics onscreen
void RenderScreen(){
  int mine_inc = 0;

  //Adds a border to expand the sub redraw zone
  int redraw_border = 3;
    
  
  //Clear the full screen
  SDL_FillRect(screen, NULL, bgColor);

  //Calculate the clear mine rects
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){

    clear_mine[mine_inc].top =  mine[mine_inc].prev_y;
    if(clear_mine[mine_inc].top < 0)
    clear_mine[mine_inc].top = 0;

    clear_mine[mine_inc].left =  mine[mine_inc].x + mine[mine_inc].width;
    if(clear_mine[mine_inc].left < 0)
    clear_mine[mine_inc].left = 0;

    clear_mine[mine_inc].right = mine[mine_inc].prev_x + mine[mine_inc].width;
    if(clear_mine[mine_inc].right < 0)
    clear_mine[mine_inc].right = 0;

    clear_mine[mine_inc].bottom = mine[mine_inc].prev_y + mine[mine_inc].height;
    if(clear_mine[mine_inc].bottom < 0)
    clear_mine[mine_inc].bottom = 0;

  }

  //The sub has moved to the right
  if(sub.x > sub.prev_x){
    clear_sub.left = sub.prev_x;
    clear_sub.right =  sub.x;
    clear_sub.top = sub.y;
    clear_sub.bottom = sub.y + PLAYER_HEIGHT;
  }
  
  //The sub has moved to the left
  if(sub.x < sub.prev_x){
    clear_sub.left = sub.x+PLAYER_WIDTH;
    clear_sub.right = sub.prev_x+PLAYER_WIDTH;
    clear_sub.top = sub.y;
    clear_sub.bottom = sub.y + PLAYER_HEIGHT;
  }
  
  
  //The sub has moved down
  if(sub.y > sub.prev_y){
    clear_sub.left =  sub.x;
    clear_sub.right =  sub.x+PLAYER_WIDTH;
    clear_sub.top = sub.prev_y;
    clear_sub.bottom =  sub.y;
  }

  //The sub has moved up
  if(sub.y < sub.prev_y){
    clear_sub.left =  sub.x;
    clear_sub.right =  sub.x+PLAYER_WIDTH;
    clear_sub.top = sub.y + PLAYER_HEIGHT;
    clear_sub.bottom = sub.prev_y + PLAYER_HEIGHT;
  }


  //Make sure the top left of the sub clear rect is positive
  if (clear_sub.left <=0)
  clear_sub.left = 0;

  //Keep sub clear rect below status text
  if (clear_sub.top <= STATUS_TEXT_CELL_HEIGHT)
  clear_sub.top = STATUS_TEXT_CELL_HEIGHT;

  //Set up the clear rect for the torpedo
  clear_torpedo.left = torpedo.prev_x-1;
  clear_torpedo.top =  torpedo.prev_y-1;
  clear_torpedo.right =  torpedo.x;
  clear_torpedo.bottom =  torpedo.y+torpedo.height+1;


  //Make sure the top left of the torpedo clear rect is positive
  if (clear_torpedo.left <=0)
  clear_torpedo.left = 0;

  if (clear_torpedo.top <=0)
  clear_torpedo.top = 0;


  //Set up the bonus block clear rect
  clear_bonus_block.top =  bonus_block.prev_y;
  if(clear_bonus_block.top < 0)
  clear_bonus_block.top = 0;

  clear_bonus_block.left =  bonus_block.x + bonus_block.width;
  if(clear_bonus_block.left < 0)
  clear_bonus_block.left = 0;

  clear_bonus_block.right = bonus_block.prev_x + bonus_block.width;
  if(clear_bonus_block.right < 0)
  clear_bonus_block.right = 0;

  clear_bonus_block.bottom = bonus_block.prev_y + bonus_block.height;
  if(clear_bonus_block.bottom < 0)
  clear_bonus_block.bottom = 0;

  /*
  //Only clear the sub rect if the sub has moved
  if( (sub.prev_x != sub.x)|| (sub.prev_y != sub.y) || (sub.x < (0)) || (sub.y < (STATUS_TEXT_CELL_HEIGHT)) ){
    //clear the rectangle of the old mini-sub sprite
  sub_clear_rect.x = clear_sub.left;
  sub_clear_rect.y = clear_sub.top;
  sub_clear_rect.w = clear_sub.right-clear_sub.left;
  sub_clear_rect.h = clear_sub.bottom-clear_sub.top;
  
  SDL_FillRect(screen, &sub_clear_rect, bgColor);
  }


  //Clear the rectangle of the old sea mine sprites
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){
  mine_clear_rect.x = clear_mine[mine_inc].left;
  mine_clear_rect.y = clear_mine[mine_inc].top;
  mine_clear_rect.w = clear_mine[mine_inc].right-clear_mine[mine_inc].left;
  mine_clear_rect.h = clear_mine[mine_inc].bottom-clear_mine[mine_inc].top;
  
  SDL_FillRect(screen, &mine_clear_rect, bgColor);
  }
  */

  /*
  //Clear the bonus block rect if the bonus block x position is onscreen
  if(bonus_block.x <= screen_width){
    //Redraw the screen
  bonus_block_clear_rect.x = clear_bonus_block.left;
  bonus_block_clear_rect.y = clear_bonus_block.top;
  bonus_block_clear_rect.w = clear_bonus_block.right-clear_bonus_block.left;
  bonus_block_clear_rect.h = clear_bonus_block.bottom-clear_bonus_block.top;

  SDL_FillRect(screen, &bonus_block_clear_rect, bgColor);
  }



  //Clear the rectangle of the old torpedo
  if(fire_torpedo_flag){
  torpedo_clear_rect.x = clear_torpedo.left;
  torpedo_clear_rect.y = clear_torpedo.top;
  torpedo_clear_rect.w = clear_torpedo.right-clear_torpedo.left;
  torpedo_clear_rect.h = clear_torpedo.bottom-clear_torpedo.top;
  
  SDL_FillRect(screen, &torpedo_clear_rect, bgColor);
  }

*/

//Slowly pan the Mini-Sub game title offscreen when the game starts
if(title_origin_x > -600){

	//Hold the title still for 90 frames
	if(frame_counter > 90){
		//Pan the text to the left
		title_origin_x -= mine_speed/3;
	}
	TFT_Write_Big_Text("Mini-Sub Game", title_origin_x, (screen_height/2)-72);
	
	TFT_Write_Text("By Andrew Hazelden", title_origin_x+150, (screen_height/2));
	
	//Debug title scrolling position
	//printf("title x: %d\n", title_origin_x);
}



  //--------------------------
  //Draw the sprites onscreen
  //--------------------------

  //Draw the sea mine images
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){
    //Animate the seamines
    AnimateSeaMine(mine_inc);
  }

  //Draw the bonus block if the bonus block x position is onscreen
  if(bonus_block.x <= screen_width){
    //Decide which bonus to display
    
    bonus_block_rect.x = bonus_block.x;
    bonus_block_rect.y = bonus_block.y;
    //bonus_block_rect.w = heart_pack_bmp->w;
    //bonus_block_rect.h = heart_pack_bmp->h;
    
    if(bonus_type == 2) {
      //Set the bonus to be an extra life
      SDL_BlitSurface(heart_pack_bmp, NULL, screen, &bonus_block_rect);

    }
    else{
      //Set the bonus to be a torpedo pack
      SDL_BlitSurface(torpedo_pack_bmp, NULL, screen, &bonus_block_rect);
    }
  }

  //Draw the torpedo if it has been fired
  if(fire_torpedo_flag){
    torpedo_rect.x = torpedo.x;
    torpedo_rect.y = torpedo.y;
    SDL_BlitSurface(torpedo_bmp, NULL, screen, &torpedo_rect);
  }


  //Draw the mini-sub image
  sub_rect.x = sub.x;
  sub_rect.y = sub.y;
  SDL_BlitSurface(sub_bmp, NULL, screen, &sub_rect);

  //Debug draw the test black color to view the sub clear rect
  //TFT_Set_Brush(1, CL_BLACK, 0, TOP_TO_BOTTOM, CL_BLACK, CL_BLACK);

  /*
  //Only clear the sub rect if the sub has moved
  if( (sub.prev_x != sub.x)|| (sub.prev_y != sub.y) || (sub.x < (0)) || (sub.y < (STATUS_TEXT_CELL_HEIGHT)) ){
    //clear the rectangle of the old mini-sub sprite
    sub_clear_rect.x = clear_sub.left;
  sub_clear_rect.y = clear_sub.top;
  sub_clear_rect.w = clear_sub.right-clear_sub.left;
  sub_clear_rect.h = clear_sub.bottom-clear_sub.top;

  SDL_FillRect(screen, &sub_clear_rect, bgColor);
  }
*/

  //SDL_Flip(screen);

  //Scan the sea mines array to check for a mine hit by the sub
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){
    //Check for a mini-sub to sea mine collision
    if(IsCollision(sub.x, sub.y, PLAYER_WIDTH, PLAYER_HEIGHT, mine[mine_inc].x, mine[mine_inc].y, mine[mine_inc].width, mine[mine_inc].height)) {
      //Set a sea mine hit flag
      mineHit = 1;
      
      //Set the id number for the destroyed sea mine
      destroyed_mine_id = mine_inc;

      //debug which mine blew up
      UART1_Write_Line("You hit a sea mine!");
      //UART1_Write_Line("Mine hit:");
      //UART1_Write_Variable(mine_inc);
    }

    if(fire_torpedo_flag){
      //Check for a torpedo to sea mine collision
      if(IsCollision(torpedo.x, torpedo.y, torpedo.width, torpedo.height, mine[mine_inc].x, mine[mine_inc].y, mine[mine_inc].width, mine[mine_inc].height)) {
        
        //Increment the mine hit counter
        mine[mine_inc].hits++;
        
        //Destroy a red sea mine on the first hit
        if(mine[mine_inc].type == 0){
          //Set a sea mine hit flag
          mineExplode = 1;
          
          //Set the id number for the destroyed sea mine
          destroyed_mine_id = mine_inc;

          //Debug which mine blew up
          UART1_Write_Line("Your torpedo destroyed a red sea mine!");
        }
        else if( (mine[mine_inc].type == 1) && (mine[mine_inc].hits >= 2) ){
          //Destroy a grey sea mine if it has been hit twice
          mineExplode = 1;
          
          //Set the id number for the destroyed sea mine
          destroyed_mine_id = mine_inc;

          //Debug which mine blew up
          UART1_Write_Line("Your torpedo destroyed a grey sea mine!");
        }
        else{
          //A grey sea mine was hit a single time
          
          //Reset torpedo launch flag
          fire_torpedo_flag = 0;
          
          //Clear the torpedo sprite
          ClearTorpedo();

          //Reset the torpdedo after the hit
          torpedo.x = 0;
          torpedo.y = 0;
          torpedo.prev_x = 0;
          torpedo.prev_y = 0;
          
        }
      }
    }
  }


  //Check for a mini-sub to bonus block collision
  if(IsCollision(sub.x, sub.y, PLAYER_WIDTH, PLAYER_HEIGHT, bonus_block.x, bonus_block.y, bonus_block.width, bonus_block.height)) {

    //Set the id number for the bonus
    bonus_id = 1;
    
    
    if(bonus_type == 2){
      //The sub just picked up an extra life bonus
      subLives += 1;
      
      //debug which bonus was picked up
      UART1_Write_Line("You picked up an extra life bonus!");
    }
    else{
      //The sub just picked up 3 more torpedos
      torpedo_count += 3;
      
      //debug which bonus was picked up
      UART1_Write_Line("You picked up a torpedo 3 pack bonus!");
    }

    
    //reset the bonus block position
    ResetBonusBlock();
    
    //Fill the screen with the blue sea color
    //SDL_FillRect(screen, NULL, bgColor);
    //SDL_Flip(screen);
  }

  
  //Draw the fireball explosion sprite when the sub collides with a sea mine
  if(mineHit){
    //Your sub was destroyed

    //Center the fireball on the sub
    fireball.x = (sub.x + (PLAYER_WIDTH/2) ) - (fireball.width/2) ;
    fireball.y = (sub.y + (PLAYER_HEIGHT/2) ) - (fireball.height/2) ;

    //Keep the fireball from going off the top or left side of the screen
    if( fireball.x <=0)
    fireball.x=0;

    if( fireball.y <= STATUS_TEXT_CELL_HEIGHT)
    fireball.y = STATUS_TEXT_CELL_HEIGHT;
    
    fireball_rect.x = fireball.x;
    fireball_rect.y = fireball.y;
    
    //Show fireball image
    SDL_BlitSurface(fireball_bmp, NULL, screen, &fireball_rect);

    //SDL_Flip(screen);
    //SDL_Delay(500);
    //Fill the screen with the blue sea color
    //SDL_FillRect(screen, NULL, bgColor);

  }



  //Draw the fireball explosion sprite when the torpedo blows up a sea mine
  if(mineExplode){
    //A torpedo blew up a sea mine

    //Status debugging
    //UART1_Write_Line("torpedo.x:");
    //UART1_Write_Variable(torpedo.x);
    
    //Clear the torpedo sprite
    ClearTorpedo();
    
    //Center the explosion on the destroyed seamine
    explode.x = (mine[destroyed_mine_id].x + (mine[destroyed_mine_id].width/2) ) - (explode.width/2) ;
    explode.y = (mine[destroyed_mine_id].y + (mine[destroyed_mine_id].height/2) ) - (explode.height/2);

    //Keep the fireball from going off the top or left side of the screen
    if( explode.x <=0 ){
      explode.x=0;
    }

    if( explode.y <= STATUS_TEXT_CELL_HEIGHT){
      explode.y = STATUS_TEXT_CELL_HEIGHT;
    }

    //Show fireball image
    explode_rect.x = explode.x;
    explode_rect.y = explode.y;
    
    //Show fireball image
    SDL_BlitSurface(explode_bmp, NULL, screen, &explode_rect);
    //SDL_Flip(screen);
    //SDL_Delay(100);
    //Fill the screen with the blue sea color
    //SDL_FillRect(screen, NULL, bgColor);

    //Reset mineExplode flag
    mineExplode = 0;

    //Reset torpedo launch flag
    fire_torpedo_flag = 0;

    //Reset the torpdedo after explosion
    torpedo.x = 0;
    torpedo.y = 0;
    torpedo.prev_x = 0;
    torpedo.prev_y = 0;

    //Reset the mine position after collision
    //Draw the new mine position staring on the right edge of the screen
    ResetSeaMine(destroyed_mine_id);
    mine[destroyed_mine_id].prev_x = mine[destroyed_mine_id].x;

    //Increase the score
    if(mine[destroyed_mine_id].type == 0){
      score += 50;
    }
    else if(mine[destroyed_mine_id].type == 1){
      score += 100;
    }
  }

  //Check if the current level is over
  CheckGameLevel();

  //Display the Score in the status bar
  RenderScore();

  //Update screen	
  SDL_Flip(screen);

	//Delay the explosion onscreen for 1/10th of a second 
	if(mineHit || mineExplode){
		SDL_Delay(100);
	}

}


void RenderScore(){
  
  
  
  int tr_inc = 0;
  int life_inc = 0;
  
  //Position the torpedo count icons at 80% of the screen width 
  int torpdeo_screen_origin = (float)(screen_width*0.80);
  
  //Position the life count icons at 1.5% of the screen width 
  int lives_screen_origin =  (float)(screen_width*0.0156);

  //Display the Score in the status bar


  //Redraw the background behind the text if the values have changed
  //if( (score != prev_score) ||  (prev_torpedo_count != torpedo_count) || (prev_subLives != subLives) ||   ( prev_muteSound != muteSound)){
  
  //Blank the score background
  //SDL_FillRect(screen, &score_clear_rect, bgColor);
  
  //Draw the sub lives and the remaining torpedos icons
  
  //Torpedos remaining icons
  for(tr_inc=0;tr_inc<torpedo_count;tr_inc++){
    torpedo_status_rect.x = torpdeo_screen_origin+((torpedo_status_bmp->w+2)*tr_inc);
    SDL_BlitSurface(torpedo_status_bmp, NULL, screen, &torpedo_status_rect);
  }
  
  
  //Lives remaining icons
  
  //Single heart icon
  //lives_rect.x = lives_screen_origin;
  //SDL_BlitSurface(lives_bmp, NULL, screen, &lives_rect);
  
  //Show a row of heart icons
  for(life_inc=0;life_inc<subLives;life_inc++){
    lives_rect.x = lives_screen_origin+((lives_bmp->w+2)*life_inc);
    SDL_BlitSurface(lives_bmp, NULL, screen, &lives_rect);
  }
  
  
  if(muteSound){
    //Sound off (muted) icon
    SDL_BlitSurface(soundMute_bmp, NULL, screen, &sound_rect);
  }
  else if(!muteSound){
    
    //if (sound_level > 75) {
    //Sound on icon
    SDL_BlitSurface(sound_bmp, NULL, screen, &sound_rect);
    //}
    //else if (sound_level > 40){
    //Sound on icon
    //TFT_Image(80, 2, sound_med_bmp, 1);
    //}
    //else{
    //Sound on icon
    //TFT_Image(80, 2, sound_low_bmp, 1);
    //}
    
  }
  
  
  
  //Convert the level number from an int to string
  IntToStr(level, level_text);
  
  //Convert the sub lives from an int to string
  IntToStr(subLives, lives_text);
  
  //Convert the torpedoes from an int to string
  IntToStr(torpedo_count, torpedoes_text);
  
  //Convert the score from an int to string
  IntToStr(score, score_text);
  
  
  //Write the lives remaining text onscreen
  //TFT_Write_Text(lives_text, (lives_screen_origin + 20), 2);
  
  //Write the torpedoes remaining text onscreen
  //TFT_Write_Text(torpedoes_text, (torpdeo_screen_origin + 35), 2);
  
  //Create the score string
  //strcpy(score_display_text, "Score: ");
  strcpy(score_display_text, "Score:  ");
  strcat(score_display_text, score_text);
  strcat(score_display_text, "\0");
  
  //Write the score onscreen
  //TFT_Write_Text(score_display_text, screen_width/2, 2);
  PrintCenteredTextLine(score_display_text, 2);
  
  //Print the status text every 90 frames
  if( (frame_counter % 90) == 0 ){
    printf("Lives: %s Torpedoes: %s Score: %s\n", lives_text, torpedoes_text, score_text);
  }
  
}



void CheckGameLevel(){
  //Set the game level
  if( (level == 1) && (score > 1000) ){
    level = 2;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 8;

    //The bonus frequency factor
    bonus_factor = 1;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 5;

  }

  if( (level == 2) && (score > 2500) ){
    level = 3;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 8;

    //The bonus frequency factor
    bonus_factor = 1;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 4;

    //give an extra sub life
    subLives++;
  }

  if( (level == 3) && (score > 5000) ){
    level = 4;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 8;

    //The bonus frequency factor
    bonus_factor = 2;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 3;

    //give an extra sub life
    subLives++;
  }

  if( (level == 4) && (score > 8000) ){
    level = 5;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 9;

    //The bonus frequency factor
    bonus_factor = 3;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 3;

    //give an extra sub life
    subLives++;
  }

  if( (level == 5) && (score > 10000) ){
    level = 6;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 10;

    //The bonus frequency factor
    bonus_factor = 3;
    
    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 3;

    //give an extra sub life
    subLives++;
  }

  if( (level == 6) && (score > 12000) ){
    level = 7;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 10;

    //The bonus frequency factor
    bonus_factor = 3;
    
    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 3;

    //give an extra sub life
    subLives++;
  }

  if( (level == 7) && (score > 15000) ){
    level = 8;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 10;

    //The bonus frequency factor
    bonus_factor = 4;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 3;

    //give an extra sub life
    subLives++;
  }

  if( (level == 8) && (score > 15000) ){
    level = 9;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 10;

    //The bonus frequency factor
    bonus_factor = 3;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 2;

    //give an extra sub life
    subLives++;
  }
  if( (level == 9) && (score > 20000) ){
    level = 10;
    prev_level = level-1;

    //The mine scrolling speed
    mine_speed = 10;

    //The bonus frequency factor
    bonus_factor = 4;

    //The rate from 1 to 5 of a grey sea mine occuring
    grey_mine_rate = 2;

    //give an extra sub life
    subLives++;
  }


  if (prev_level != level){
    //Switch to the next level
    ShowNextLevel();
  }

}





//Erase the exploded torpedo
void ClearTorpedo(){
  //Set up the clear rect for the exploded torpedo
  torpedo_clear_rect.x = torpedo.x;
  torpedo_clear_rect.y = torpedo.y;
  torpedo_clear_rect.w = torpedo.width;
  torpedo_clear_rect.h = torpedo.height;

  //Erase the torpedo from the screen
 // SDL_FillRect(screen, &torpedo_clear_rect, bgColor);
  
}




//Check for a sprite to mine collision
char IsCollision (unsigned int Shape_X, unsigned int Shape_Y,  unsigned int Shape_Width, unsigned int Shape_Height,
unsigned int Bomb_Left, unsigned int Bomb_Top, unsigned int Bomb_Width, unsigned int Bomb_Height) {
  
  //Check for sea mine sidewall collisions
  //The right side of Shape is greater than left side of Bomb     and        left side of Shape is smaller than right side of Bomb
  if( ((Shape_X + Shape_Width) >= Bomb_Left )               &&              ((Shape_X) <= (Bomb_Left + Bomb_Width-1)) &&
      
      
      //Check for sea mine top and bottom collisions 
      //The bottom side of Shape is greater than top of Bomb         and            top of Shape is smaller then bottom of Bomb
      ((Shape_Y + Shape_Height) >= Bomb_Top )                   &&               ((Shape_Y) <= (Bomb_Top + Bomb_Height-1))  ) {
    
    
    //Debug printing
    //UART1_Write_Line("Collision detected");
    
    //A collision has been detected
    return 1;
  }
  else
  {
    //Default return value
    //No collisions have been detected
    return 0;
  }
  
}




//Animate the Sea Mine sprites
void AnimateSeaMine(int mineNum){
  unsigned long the_frame = 0;
  
  SDL_Rect animate_mine_rect;

  //Calculate the animated sea mine frame number
  the_frame = (frame_counter + mineNum) % 6;
  
  //Animate the mine rotation every nth frame
  //if(frame_counter % 2 == 0){
  //	the_frame = (frame_counter + mineNum) % 6;
  //}

  animate_mine_rect.x = mine[mineNum].x;
  animate_mine_rect.y = mine[mineNum].y;
  animate_mine_rect.w = mine1_bmp->w;
  animate_mine_rect.h = mine1_bmp->h;
  
  
  //Debug: test rendering with a single mine color
  //SDL_BlitSurface(mine6_bmp, NULL, screen, &animate_mine_rect);
  
 
  //If the seamine.type equals zero it is a red mine
  if(mine[mineNum].type == 0) {

    //Select the red sea mine sprite
    switch (the_frame) {
    case 0 :
      SDL_BlitSurface(mine6_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 1 :
      SDL_BlitSurface(mine5_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 2 :
      SDL_BlitSurface(mine4_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 3 :
      SDL_BlitSurface(mine3_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 4 :
      SDL_BlitSurface(mine2_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 5 :
      SDL_BlitSurface(mine1_bmp, NULL, screen, &animate_mine_rect);
      break;
    default:
      SDL_BlitSurface(mine1_bmp, NULL, screen, &animate_mine_rect);
      break;
    }
    
  }
  else{
    
    //Select the grey sea mine sprite
    switch (the_frame) {
    case 0 :
      SDL_BlitSurface(dark_mine6_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 1 :
      SDL_BlitSurface(dark_mine5_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 2 :
      SDL_BlitSurface(dark_mine4_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 3 :
      SDL_BlitSurface(dark_mine3_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 4 :
      SDL_BlitSurface(dark_mine2_bmp, NULL, screen, &animate_mine_rect);
      break;
    case 5 :
      SDL_BlitSurface(dark_mine1_bmp, NULL, screen, &animate_mine_rect);
      break;
    default:
      SDL_BlitSurface(dark_mine1_bmp, NULL, screen, &animate_mine_rect);
      break;
    }
      
  } 

}


//Save the previous sprite positions
void SavePreviousVal(){
  int mine_inc = 0;
  
  //Save the previous score
  prev_score = score;
  
  //Previous sub position
  sub.prev_x = sub.x;
  sub.prev_y = sub.y;
  
  //Previous sea mine position
  for(mine_inc = 0; mine_inc <= (NUMBER_OF_SEA_MINES-1); mine_inc++){
    mine[mine_inc].prev_x = mine[mine_inc].x;
    mine[mine_inc].prev_y = mine[mine_inc].y;
  }
  
  //Previous torpedo position
  torpedo.prev_x = torpedo.x;
  torpedo.prev_y = torpedo.y;
  
  //Previous bonus block position
  bonus_block.prev_x = bonus_block.x;
  bonus_block.prev_y = bonus_block.y;
  
  //Previous values for refreshing the status screen
  prev_torpedo_count = torpedo_count;
  prev_subLives = subLives;
  
  //Reset the destroyed sea mine id number
  destroyed_mine_id = -1;
  
  //Reset the bonus id
  bonus_id = -1;
  
  //Save previous level
  prev_level = level;
  
  //previous sound on / off setting
  prev_muteSound = muteSound;
  
}
/* ----------------------------------------- */

//Write text to the SDL screen
int TFT_Write_Text(char *textstring,int x_pos, int y_pos){
  SDL_Color text_color;
  
  //White text
  //text_color.r = 255;
  //text_color.g = 255;
  //text_color.b = 255;
  
  //Yellow text
  text_color.r = 239;
  text_color.g = 189;
  text_color.b = 0;
  
  
  SDL_Surface *textSurface = TTF_RenderText_Blended(fnt, textstring, text_color);
  
  SDL_Rect textDestRect;
  
  if (textSurface) {
    /* now center it horizontally and position it just below the logo */
    textDestRect.x = x_pos;
    textDestRect.y = y_pos;
    textDestRect.w = textSurface->w;
    textDestRect.h = textSurface->h;
    SDL_BlitSurface(textSurface, NULL, screen, &textDestRect);
    SDL_FreeSurface(textSurface);
  }
  //SDL_Flip(screen);	
  return textDestRect.w;
}

//Write Big text to the SDL screen
int TFT_Write_Big_Text(char *textstring, int x_pos, int y_pos){
  SDL_Color text_color;
  
  //yellow text
  text_color.r = 239;
  text_color.g = 189;
  text_color.b = 0;
  
  SDL_Surface *textSurface = TTF_RenderText_Blended(fnt_big, textstring, text_color);
  
  SDL_Rect textDestRect;
  
  if (textSurface) {
    /* now center it horizontally and position it just below the logo */
    textDestRect.x = x_pos;
    textDestRect.y = y_pos;
    textDestRect.w = textSurface->w;
    textDestRect.h = textSurface->h;
    SDL_BlitSurface(textSurface, NULL, screen, &textDestRect);
    SDL_FreeSurface(textSurface);
  }
  //SDL_Flip(screen);	
  return textDestRect.w;
}


//UART1 write text and new line (carriage return + line feed)
void UART1_Write_Line(char *uart_text) {
  printf("%s\n", uart_text);
}

//UART1 write integer variable and new line (carriage return + line feed)
void UART1_Write_Variable(int var) {
  printf("%d\n", var);
}

//UART1 write long variable and new line (carriage return + line feed)
void UART1_Write_Long_Variable(long var){
  printf("%ld\n", var);
}



//UART1 write label and variable (carriage return + line feed)
void UART1_Write_Label_Var(char *uart_text, int var ) {
  printf("%s %d\n", uart_text, var);
}


//UART1 write label and long variable (carriage return + line feed)
void UART1_Write_Label_Long_Var(char *uart_text, long var){
  printf("%s %ld\n", uart_text, var);
}

//UART1 write label and float variable (carriage return + line feed)
void UART1_Write_Label_Float_Var(char *uart_text, float var){
  printf("%s %lf\n", uart_text, var);
}


/* ----------------------------------------- */


void IntToStr(int input, char *output){
  sprintf(output,"%0d", input);
}

