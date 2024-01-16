#include <iostream>
#include <sstream>
#include <random>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
using namespace std;

#define ScreenWidth 800
#define ScreenHeight 600

const float deltaTime = 1.0 / 60.0;

struct vec2 {
    float x, y;
    
    // Default constructor (initialize to zero)
    vec2() : x(0.0), y(0.0) {}

    // Constructor with three arguments
    vec2(float x, float y) : x(x), y(y) {}
};

struct vec4 {
    float x1, x2, y1, y2;
    
    vec4(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

float damper(float value, float goal, float factor = 0.1) {
    return (1.0f - factor) * value + factor * goal;
}

class Square {
public:
    vec4 square;
    int squareThickness;
    Square() : square(200, 100, 500, 400), squareThickness(5) {}

    Square(vec4 square, int squareThickness = 5) : square(square), squareThickness(squareThickness) {}

    void draw() {
        al_draw_rectangle(square.x1, square.y1, square.x2, square.y2, al_map_rgb(255, 255, 255), squareThickness);
    }
};

class Ball : Square {
    ALLEGRO_BITMAP* player = al_load_bitmap("Bitmapa.png");
    int imageWidth = al_get_bitmap_width(player);
    int imageHeight = al_get_bitmap_height(player);
    float imageScale = 0.3;
    float imageWidthScaled = imageWidth * imageScale;
    float imageHeightScaled = imageHeight * imageScale;

    bool m_awake = true;
    float y_last = square.y1, y_now;
public:
    const float epsilon = 1.0f;
    float gravity = 0.5, energyLoss = 0.8;
    vec2 position, velocity;

    Ball() : position(300.0, 200.0), velocity(2.0, 7.0), y_now(position.y) {}

    Ball(vec2 position, vec2 velocity) : position(position), velocity(velocity), y_now(position.y) {}

    ~Ball() {
        al_destroy_bitmap(player);
    }

    void draw() {
        //al_draw_tinted_scaled_bitmap(player, al_map_rgb(random_number(), random_number(), random_number()), 0, 0, imageWidth, imageHeight, position.x, position.y, imageWidthScaled, imageHeightScaled, 0);
        al_draw_scaled_bitmap(player, 0, 0, imageWidth, imageHeight, position.x, position.y, imageWidthScaled, imageHeightScaled, 0);
    }

    void Coll() {
        position.x += velocity.x * energyLoss;
        position.y += velocity.y * energyLoss;
        velocity.y += gravity;

        // kolizja
        if (position.y + imageHeightScaled > square.y2) {
            velocity.y = -velocity.y * energyLoss;
            position.y = square.y2 - imageHeightScaled;

            if (y_now - y_last < epsilon and y_now != y_last) {
                m_awake = false;
            }

            y_last = y_now;
            y_now = position.y;
        }
        else if (position.y < square.y1) {
            velocity.y = -velocity.y * energyLoss;
        }
        else if (position.x > square.x2 - imageWidthScaled) {
            velocity.x = -velocity.x * energyLoss;
            position.x = square.x2 - imageWidthScaled;
        }
        else if (position.x < square.x1) {
            velocity.x = -velocity.x * energyLoss;
            position.x = square.x1;
        }
        if (m_awake) {
            if (position.y < y_now)
                y_now = position.y;
        }
        else {
            energyLoss = damper(energyLoss, 0, 0.01F);
        }
    }
};

int random_number() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, 255);
    return distr(gen);
}

int main() {
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon(); 

    ALLEGRO_DISPLAY* display = al_create_display(ScreenWidth, ScreenHeight);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(deltaTime);

    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_display_event_source(display));

    ALLEGRO_FONT* font = al_load_font("OpenSans-Regular.ttf", 36, 0);
    std::ostringstream text;

    vec4 square(200, 100, 500, 400);
    int squareThickness = 5;

    vec2 position(300.0, 200.0);
    vec2 velocity(2.0, 7.0);

    Square box(square, squareThickness);
    Ball ball(position, velocity);

    al_start_timer(timer);
    bool done = false;
    while (!done) {
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);

        if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        else if (events.type == ALLEGRO_EVENT_TIMER) {
            ball.Coll();

            box.draw();
            ball.draw();

            text << "gravity: " << ball.gravity;
            al_draw_text(font, al_map_rgb(255, 255, 255), square.x2 - 100, square.y2, ALLEGRO_ALIGN_LEFT, text.str().c_str());
            text.str("");
            text << "energyLoss: " << ball.energyLoss;
            al_draw_text(font, al_map_rgb(255, 255, 255), square.x2 - 100, square.y2 + 40, ALLEGRO_ALIGN_LEFT, text.str().c_str());
            text.str("");

            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
        }
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);

    return 0;
}