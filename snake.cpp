/*
Programmer: Gaddiel Morales
Program: Snake
Purpose: Showcase a user app making use of input device and GPU
*/

/*TODO list
    * draw board on rectangle
    * send rectangle to the OS
*/

#include "rand.h"
#include "sage.h"
#include "event.h"
#include "graphics.h"
#include "printf.h"

int game_height = 50, game_width = 50, scale = 50;
#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (800)

enum collisionType {none, 
        food, 
        self
    };

enum Direction{
    up, 
    right,
    down,
    left,
    stop
};

enum Color{
    red,
    green
};

class World{
    private:
        int apple_lim = 3;

        //used to find items
        SnakeHead* snake = nullptr;
        Apple** apples;

    public:
        World(){
            apples = new Apple*[apple_lim];
        }

        void setAppleLimit(int new_lim){
            this->apple_lim = new_lim;
        }

        int checkCollisions(){
           

            //check apples
            for(int i = 0; i < apple_lim; i++){
                if(snake->getX() == apples[i]->x && snake->getY() == apples[i]->y){
                    return collisionType(food);
                }
            }

            //check self
            SnakeSegment* curr_segment = snake->getNext();
            while(curr_segment){
                if(snake->getX() == curr_segment->getX() && snake->getY() == curr_segment->getY())
                return collisionType(self);
                curr_segment = curr_segment->getNext();
            }

            return collisionType(none);

        }

        void cleanBoard(){
            for(int i; i < apple_lim; i++){
                delete apples[i];
            }
            delete[] apples;
            this->snake->kill();
        }

        void spawnSnake(){
            int x = rand() % game_width;
            int y = rand() % game_height;

            this->snake = new SnakeHead(x,y,game_width,game_height);

        }

        void spawnApples(){
            if(apples == nullptr){
                apples = new Apple*[apple_lim];
            }

            for(int i = 0; i < apple_lim; i++){
                if(apples[i] == nullptr){
                    continue;
                }

                //choose random spot
                int x = rand() % game_width;
                int y = rand() % game_height;
                bool valid = true;

                //create apple if spot is valid
                //apples check
                for(int j = 0; j < apple_lim; j++){
                    if(apples != nullptr && apples[j]->x == x && apples[j]->y == y) valid = false;
                }

                //snake check
                SnakeSegment* curr_segment = snake->getNext();
                while(curr_segment){
                    if(x == curr_segment->getX() && y == curr_segment->getY())
                    valid = false;
                    curr_segment = curr_segment->getNext();
                }

                //spawn
                if(valid){
                    apples[i] = new Apple(x,y);
                }
            }

        }

        SnakeHead* getSnake(){
            return this->snake;
        }

        ~World(){
            for(int i; i < apple_lim; i++){
                delete apples[i];
            }
            delete[] apples;
            this->snake->kill();
        }

        //draws the world to the GPU
        void draw(){

            for(int i; i < apple_lim; i++){
                render(apples[i]->x,apples[i]->y, Color::green);
            }
            
            SnakeSegment* curr_segment = snake;
            while(curr_segment){
                //draw segment
                render(curr_segment->getX(),curr_segment->getY(), Color::green);
                curr_segment = curr_segment->getNext();
            }
        }
};





class Apple{
    public:
        int x, y;
        Apple(int x,int y){
            this->x = x;
            this->y = y;
        }
};

class SnakeSegment{
    protected:
        int x, y;
        bool spawned = true;   //used to tell if this segment was made on the current timestep
        //next segment is a linked segment closer to the tail, previous is a linked segment closer to the head
        SnakeSegment* next = nullptr;
        SnakeSegment* prev = nullptr;

    public:
        SnakeSegment(){
            this->x = 0;
            this->y = 0;
        }

        SnakeSegment(int x, int y){
            this->x = x;
            this->y = y;
        }

        SnakeSegment* getNext(){
            return this->next;
        }

        SnakeSegment* getPrev(){
            return this->prev;
        }

        int getX(){
            return this->x;
        }

        int getY(){
            return this->y;
        }

        //inch the snake forward, call from SnakeHead
        void moveSegment(){
            //move trailing segment to this position
            if(this->next) this->next->moveSegment();
            //move this segment to up towards head
            this->y = prev->y;
            this->x = prev->x;
        }

        //use when eating food
        void addSegment(){
            if(this->getNext()){
                this->getNext()->addSegment();
            }
            else{
                this->next = new SnakeSegment(this->x,this->y);
            }
        }

        //deletes this and trailing segments
        void kill(){
            if(this->getNext()) this->getNext()->kill();

            delete this;
            
        }

};

class SnakeHead : public SnakeSegment{
    private:
        int x_lim = 50, y_lim = 50;
    public: 

        

        Direction direction = Direction::stop;

        SnakeHead(int x, int y, int x_lim, int y_lim){
            this->x = x;
            this->y = y;
            this->next = nullptr;
            this->prev = nullptr;
            this->x_lim = x_lim;
            this->y_lim = y_lim;
        }
        
        void setDirection(int dir){
            this->direction = Direction(dir);
        }

        void moveSnake(){
            //don't move freshly spawned segments
            if(spawned){
                spawned = false;
                return;
            }

            //move trailing segment to this position
            if(this->next) this->next->moveSegment();

            //move this segment
            switch(this->direction){
                case Direction::up:
                    this->y = ((this->y + 1) % y_lim);
                    break;
                case Direction::down:
                    this->y = ((this->y - 1 + y_lim) % y_lim);
                    break;
                case Direction::left:
                    this->x = ((this->x - 1 + x_lim) % x_lim);
                    break;
                case Direction::right:
                    this->x = ((this->x + 1) % x_lim);
                    break;
            }

        }
};

Rectangle screen_rect = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};

void clearScreen() {

    // black screen
    Pixel pixBuffer[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
    Rectangle rect;
  
    pixBuffer[0].r = 0;
    pixBuffer[0].g = 0;
    pixBuffer[0].b = 0;
    pixBuffer[0].a = 0xFF;
    for (uint32_t i = 1; i < sizeof(pixBuffer) / sizeof(pixBuffer[0]); i++) {
        pixBuffer[i] = pixBuffer[0];
    }

    screen_draw_rect(pixBuffer, &rect, 1, 1);
    screen_flush(&rect);
}

void render(int x_offset, int y_offset, Color color){
    Pixel square;
    //select color
    if(color == Color::green){
        square = {0,255,0,255};
    }
    else{   //red
        square = {255,0,0,255};
    }

    //draw to buffer
    Pixel pixBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint32_t start_idx = (y_offset*(SCREEN_WIDTH/game_width)) + (x_offset * (SCREEN_HEIGHT/game_height));
    for (uint32_t i = 0; i < SCREEN_HEIGHT/game_height; i++) { //for each row
        for (uint32_t j = 0; j < SCREEN_WIDTH/game_width; j++) { //for each column
            if(start_idx + (i*SCREEN_WIDTH + j) < SCREEN_WIDTH * SCREEN_HEIGHT) //saftey measure
                pixBuffer[start_idx + (i*SCREEN_WIDTH + j)] = square;
            else
                printf("drawing out of bounds!");
        }
    }

    //send to GPU
    screen_draw_rect(pixBuffer, &screen_rect, 1, 1);
    screen_flush(&screen_rect);

}

int main(){

    //OS structures
    VirtioInputEvent* keypress;
    
    screen_get_dims(&screen_rect);
    Pixel pixBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    //time tracking
    static uint64_t game_time;
    static uint64_t snake_timer;
    static uint64_t snake_speed;
    static uint64_t difficulty_timer;

    //create board
    World snake_game = World();
    game_time = snake_timer = get_time();
    snake_speed = 1000;

    //spawn snake
    snake_game.spawnSnake();
            
    //spawn food
    snake_game.spawnApples();

    //game loop
    bool alive = true;
    while(alive){
        uint64_t current_time = get_time();

        //check inputs
        struct virtio_input_event event;

        while (get_events(&event, 1)) {
            // Check if the event is a key press
            if (event.type == EV_KEY) {
                // Handle different keys
                switch (event.code) {
                    case KEY_UP:
                        // Handle up arrow key
                        snake_game.getSnake()->setDirection(Direction::up);
                        break;
                    case KEY_DOWN:
                        // Handle down arrow key
                        snake_game.getSnake()->setDirection(Direction::down);
                        break;
                    case KEY_LEFT:
                        // Handle up arrow key
                        snake_game.getSnake()->setDirection(Direction::left);
                        break;
                    case KEY_RIGHT:
                        // Handle down arrow key
                        snake_game.getSnake()->setDirection(Direction::right);
                        break;
                }
            }
        }

        //check if eating food or eating self
        int collision = snake_game.checkCollisions();
        if(collision == collisionType::self){
        
            //wait until keypress
            while(true){
                if(get_keyboard_event(keypress) >= 0);
            }

            //refresh game board
            snake_game.cleanBoard();
            snake_game.spawnApples();  
            snake_game.spawnSnake();
        }
        else if(collision == collisionType::food){
            snake_game.getSnake()->addSegment();
        }

        //draw frames according to timer
        if((current_time - game_time) % (1000/30) > 0){  //30fps
            snake_game.draw();
            game_time = current_time;
        }

        //movesnake
        if((current_time - snake_timer) % snake_speed > 0){

            snake_game.getSnake()->moveSnake();

        }

        //speed up snake (currently every 5 seconds)
        if((current_time - difficulty_timer) % (5000) > 0){
            snake_speed -= (snake_speed * 0.1);
            difficulty_timer = current_time;

        }

    }

}

