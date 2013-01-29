void GetArguments(int argc, char *argv[]);
void InitGFX();
void PrintVersion();
void RenderScreen();
void GetInput();
int  GetTextYPos(int row);
int PrintTextLine(char *textstring);
int PrintBigCenteredTextLine(char *textstring, int y_pos);
int PrintCenteredTextLine(char *textstring, int y_pos);
int GetScreenWidth(void);
int GetScreenHeight(void);
int GetMouseButton(void);
int GetMouseX(void);
int GetMouseY(void);
void IntToStr(int input, char *output);

typedef struct {
	int r;
	int g;
	int b;
} rgb_colors;


/* drawing colors */
rgb_colors foreground_color;
rgb_colors background_color;

/* drawing location */
int previous_x = 0;
int previous_y = 0;

/* The screen surface */
SDL_Surface *screen = NULL;
SDL_Rect screen_rect;
SDL_Rect display_area_rect;
int flags=0;
  
Uint32 screen_color;
Uint32 frame_color;
Uint32 debug_color;


TTF_Font *fnt; /* C64 Font */
TTF_Font *fnt_big; /* Big C64 Font */
int text_row = 0; /* current text row */

extern void FreeSprites(); // Free the mini-sub images
extern void LoadSprites(); // Load the mini-sub images
//  ---------------------------------------------

int upButton=0; //Up
int downButton=0; //Down
int rightButton=0; //Right
int leftButton=0; //Left

int triangleButton=0; //Triangle Up
int xButton=0; //X Down
int circleButton=0; //Circle Right
int squareButton=0; //Square Left

int startButton=0; //Start


//Mute sound flag
int muteSound = 0;
int prev_muteSound = 0;

//Bonus frequency factor
int bonus_factor = 1;

int show_main_menu = 0;
int toggled_main_menu = 0;

/* ------------------------------- */

SDL_Surface* andrew_bmp = NULL;
SDL_Surface* dark_mine1_bmp = NULL;
SDL_Surface* dark_mine2_bmp = NULL;
SDL_Surface* dark_mine3_bmp = NULL;
SDL_Surface* dark_mine4_bmp = NULL;
SDL_Surface* dark_mine5_bmp = NULL;
SDL_Surface* dark_mine6_bmp = NULL;
SDL_Surface* explode_bmp = NULL;
SDL_Surface* fireball_bmp = NULL;
SDL_Surface* gameover_bmp = NULL;
SDL_Surface* heart_pack_bmp = NULL;
SDL_Surface* lives_bmp = NULL;
SDL_Surface* mikroC_bmp = NULL;
SDL_Surface* mikroe_bmp = NULL;
SDL_Surface* mine1_bmp = NULL;
SDL_Surface* mine2_bmp = NULL;
SDL_Surface* mine3_bmp = NULL;
SDL_Surface* mine4_bmp = NULL;
SDL_Surface* mine5_bmp = NULL;
SDL_Surface* mine6_bmp = NULL;
SDL_Surface* sound_bmp = NULL;
SDL_Surface* soundMute_bmp = NULL;
SDL_Surface* sound_low_bmp = NULL;
SDL_Surface* sound_med_bmp = NULL;
SDL_Surface* sub_bmp = NULL;
SDL_Surface* title_bmp = NULL;
SDL_Surface* tonyB_bmp = NULL;
SDL_Surface* torpedo_bmp = NULL;
SDL_Surface* torpedo_pack_bmp = NULL;
SDL_Surface* torpedo_status_bmp = NULL;



SDL_Rect sub_rect;
SDL_Rect sub_clear_rect;
SDL_Rect mine_clear_rect;
SDL_Rect torpedo_rect;
SDL_Rect torpedo_clear_rect;
SDL_Rect bonus_block_rect;
SDL_Rect bonus_block_clear_rect;
SDL_Rect fireball_rect;
SDL_Rect explode_rect;
SDL_Rect lives_rect;
SDL_Rect sound_rect;
SDL_Rect torpedo_clear_rect;
SDL_Rect score_clear_rect;
SDL_Rect torpedo_status_rect;

//Switch this to an array of SDL mine rects
SDL_Rect mine_rect;

SDL_Rect title_rect;

//Blue sea background color
Uint32 bgColor = 0;

/* ------------------------------- */

#define STATUS_TEXT_HEIGHT 25

//How many lives the player starts with
#define NUMBER_OF_SUBMARINES 5

//How many torpedoes the player starts with
#define NUMBER_OF_TORPEDOES 12

//How many sea mines are on screen at once
//Note - only 4 mines are setup by default in the InitSprites() function.
#define NUMBER_OF_SEA_MINES 8

//Sprite movement speed
#define SUB_SPEED 10
#define SUB_Y_SPEED 5
#define TORPEDO_SPEED 15

int mine_speed = 4;   // slow speed 4, normal speed 6, medium speed 8  high speed 10

int title_origin_x = 0;

//Track the user score
int score = 1;
int prev_score = 1;

//How many screens apart a bonus torpedo or life box is placed
#define BONUS_SCREEN_SPACING 2

//The sub's default resting X position when
#define DEFAULT_SUB_X_POS 40


void InitSprites();
void GetInput();
void RenderScreen();
void RenderScore();
void SavePreviousVal();
void ShowGameOver();
void ResetSeaMine(int mineNumber);
void ResetBonusBlock();
void ResetGame();
int RandomHeightY();
void ShowNextLevel();
void AnimateSeaMine(int mineNum);
void ClearTorpedo();
void CheckGameLevel();
void ToggleMute();


char IsCollision (unsigned int Shape_X, unsigned int Shape_Y,  unsigned int Shape_Width, unsigned int Shape_Height,
                                  unsigned int Bomb_Left, unsigned int Bomb_Top, unsigned int Bomb_Width, unsigned int Bomb_Height);


extern void UART1_Write_Line(char *uart_text);
extern void UART1_Write_Variable(int var);
extern void UART1_Write_Long_Variable(long var);


//---------------------
// SDL Mixer Variables
//---------------------
Mix_Chunk *sound = NULL;		/* Pointer to our sound, in memory*/
int channel;					/* Channel on which our sound is played*/
int audio_rate = 22050;			/* Frequency of audio playback*/
Uint16 audio_format = AUDIO_S16SYS; 	/* Format of the audio we're playing*/
int audio_channels = 2;			/* 2 channels = stereo*/
int audio_buffers = 4096;		/* Size of the audio buffers in memory*/




//------------------
//    Global Variables
//------------------
struct player {
        int x;
        int y;
        int prev_x;
        int prev_y;
};

struct object {
        unsigned int width;
        unsigned int height;
        int x;
        int y;
        int prev_x;
        int prev_y;
        int visible;
        int type;
        int hits;
};

//clear rect for old sprites
struct clear_region{
        int top;
        int left;
        int bottom;
        int right;
};

struct object fireball;
struct object explode;
struct object torpedo;
struct player sub;
struct object mine[NUMBER_OF_SEA_MINES];
struct object bonus_block;


//Rectangles for clearing the old sprites between view updates
struct clear_region clear_mine[NUMBER_OF_SEA_MINES];
struct clear_region clear_sub;
struct clear_region clear_torpedo;
struct clear_region clear_bonus_block;

//Flag to track mine hits
int mineHit = 0;

//Flag to track mine explosions
int mineExplode = 0;

//The current frame number
unsigned long frame_counter = 0;

//How many lives the player has
int subLives = NUMBER_OF_SUBMARINES;
int prev_subLives = 0;

//How many torpedos the sub has
int torpedo_count = NUMBER_OF_TORPEDOES;
int prev_torpedo_count = 0;

//The torpedo launch check flag
int fire_torpedo_flag = 0;

//The id number of the destroyed sea mine
int destroyed_mine_id = -1;

//The id number of the bonus box
int bonus_id = -1;

//What type of bonus box is available
int bonus_type = 0;

//The bonuses displayed counter
int bonus_counter = 0;

//Track the current level number
int level = 0;

//Track the change in levels
int prev_level = 0;


//The rate from 1 to 5 of a grey seamine occuring
int grey_mine_rate = 5;

//Track the high score since power on
//Future - Update with eeprom highscore save / load
unsigned int high_score = 10000;

char level_text[10];
char score_text[10];
char lives_text[10];
char torpedoes_text[10];

//The final status display string
char score_display_text[80];

//The final level display string
char level_display_text[80];

//Temp debug value printing string
char temp_txt[12];

//flag the the start button has been pressed - skip the game screens
int start_game_flag = 0;
