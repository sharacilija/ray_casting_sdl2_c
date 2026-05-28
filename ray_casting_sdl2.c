#include <stdio.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846
#define DEG2RAD PI/180

#define WIDTH 900
#define HEIGHT 600

#define PLAYER_FOV 80

#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_BLUE 0x000000ff
#define COLOR_RED 0x00ff0000
#define PLAYER_COLOR 0x000000ff
#define CELL_SIZE 50

#define MAP_WIDTH 5
#define MAP_HEIGHT 5 

#define RENDER_DISTANCE 250

#define RAY_STEP_SIZE 5
#define SCENE_WIDTH MAP_WIDTH*CELL_SIZE
#define SCENE_HEIGHT MAP_HEIGHT*CELL_SIZE

#define PLAYER_ROTATION_SPEED 2
#define PLAYER_MOVEMENT_SPEED 5

#define VERTICAL_SCALE 10000

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {3,2,0,2,3},
    {3,0,0,0,3},
    {3,0,0,0,3},
    {3,0,0,0,3},
    {3,3,3,3,3}
};

typedef struct {
    double x, y, angle;
} Player;

// tells us the distance and type of box the ray collided with
typedef struct {
    double distance;
    int box_type;
} DistanceResult;

Player player = {125, 125, 45};

bool app_running = true;

void DrawPlayer(SDL_Surface *psurface, Player player)
{
    SDL_Rect player_rect = {player.x, player.y, 10, 10};
    SDL_FillRect(psurface, &player_rect, PLAYER_COLOR);
}

// returns the distance to the next wall assuming the player looks in given direction
DistanceResult GetDistance(Player player, double angle)
{
    // cast a ray
    bool wall_detected = false;

    double ray_x = player.x;
    double ray_y = player.y;

    while (!wall_detected)
    {
        double ray_distance = sqrt(
            pow(ray_x-player.x, 2) + 
            pow(ray_y-player.y, 2)
        );

        if (ray_distance > RENDER_DISTANCE)
        {
            return (DistanceResult) {-1, -1};
        }

        // which cell is the ray inside now?
        int cell_x = ray_x / CELL_SIZE;
        int cell_y = ray_y / CELL_SIZE;

        if (cell_x < 0 || cell_x >= MAP_WIDTH || 
            cell_y < 0 || cell_y >= MAP_HEIGHT)
        {
            return (DistanceResult) {-1, -1};
        }

        // is the ray inside a wall?
        double angle_rad = angle * PI / 180.0;
        if (map[cell_y][cell_x] > 0)
        {
            return (DistanceResult) {ray_distance, map[cell_y][cell_x]};
        }

        ray_x += cos(angle_rad) * RAY_STEP_SIZE;
        ray_y += sin(angle_rad) * RAY_STEP_SIZE;
    }

    printf("What happened?\n");
    exit(-1);
    return (DistanceResult) {-1, -1};
}

// draws a vertical line centered around the horizontal center axis of the window
void DrawVerticalLine(SDL_Surface *psurface, double height, double x, Uint32 color)
{
    SDL_Rect vertical_rect = {x, HEIGHT/2 - height/2, 1, height};
    SDL_FillRect(psurface, &vertical_rect, color);
}

void DrawFOV(SDL_Surface* psurface, Player player)
{
    for (int screen_x = 0; screen_x < WIDTH; screen_x++)
    {
        double angle =
            player.angle - PLAYER_FOV / 2.0 +
            ((double)screen_x / WIDTH) * PLAYER_FOV;

        DistanceResult result = GetDistance(player, angle);

        if (result.distance <= 0)
        {
            continue;
        }

        // fish-eye correction
        double angle_difference = angle - player.angle;
        result.distance *= cos(angle_difference * DEG2RAD);

        double visual_height = VERTICAL_SCALE / result.distance;

        Uint32 color = COLOR_WHITE;
        switch (result.box_type)
        {
            case 1:
                color = COLOR_WHITE;
                break;
            case 2:
                color = COLOR_BLUE;   
                break;
            case 3:
                color = COLOR_RED;
                break;
        }

        DrawVerticalLine(
            psurface,
            visual_height,
            screen_x,
            color 
        );
    }
}

void handle_key(SDL_Event event)
{
    SDL_KeyCode code = event.key.keysym.sym;
    if (code == SDLK_LEFT)
    {
        player.angle -= PLAYER_ROTATION_SPEED;
    }
    if (code == SDLK_RIGHT)
    {
        player.angle += PLAYER_ROTATION_SPEED;
    }
    if (code == SDLK_UP)
    {
        player.x += cos(player.angle*DEG2RAD)*PLAYER_MOVEMENT_SPEED;
        player.y += sin(player.angle*DEG2RAD)*PLAYER_MOVEMENT_SPEED;
    }
    if (code == SDLK_DOWN)
    {
        player.x -= cos(player.angle*DEG2RAD)*PLAYER_MOVEMENT_SPEED;
        player.y -= sin(player.angle*DEG2RAD)*PLAYER_MOVEMENT_SPEED;
    }
}

int main()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *pwindow = SDL_CreateWindow("Ray Casting in C using SDL2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow);

    while (app_running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    app_running = false;
                    break;
                case SDL_KEYDOWN:
                    handle_key(event);
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        app_running = false;
                    }
                    break;
            }
        }
        SDL_FillRect(psurface, NULL, COLOR_BLACK);

        DrawFOV(psurface, player);
        
        SDL_UpdateWindowSurface(pwindow);
        SDL_Delay(16);
    }
    
    return 0;
}
