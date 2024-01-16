#include <sstream>
#include <iostream>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

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

// Function to calculate the length/magnitude of a 3D vector
float length(const vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

float damper(float value, float goal, float factor = 0.1) {
    return (1.0f - factor) * value + factor * goal;
}


int main() {
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    // Constants for physics
    const float epsilon = 1.0f;
    float gravity = 0.5, energyLoss = 0.8;

    ALLEGRO_DISPLAY* display = al_create_display(ScreenWidth, ScreenHeight);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(deltaTime);

    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_display_event_source(display));

    ALLEGRO_FONT* font = al_load_font("OpenSans-Regular.ttf", 36, 0);
    std::ostringstream text;

    ALLEGRO_BITMAP* player = al_load_bitmap("Bitmapa.png");
    int imageWidth = al_get_bitmap_width(player);
    int imageHeight = al_get_bitmap_height(player);
    float imageScale = 0.3;
    float imageWidthScaled = imageWidth * imageScale;
    float imageHeightScaled = imageHeight * imageScale;

    int rectangleX1 = 200, rectangleX2 = 500;
    int rectangleY1 = 100, rectangleY2 = 400;
    int rectangleThickness = 5;

    vec2 position(300.0, 200.0);
    vec2 velocity(20.0, 0.0);
    bool m_awake = true;

    float y_last = rectangleY1, y_now = position.y;

    al_start_timer(timer);
    float temp;
    bool done = false;
    while (!done) {
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);

        if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        else if (events.type == ALLEGRO_EVENT_TIMER) {
            position.x += velocity.x * energyLoss;
            position.y += velocity.y * energyLoss;
            velocity.y += gravity;

            // kolizja
            if (position.y + imageHeightScaled > rectangleY2) {
                velocity.y = -velocity.y * energyLoss;
                position.y = rectangleY2 - imageHeightScaled;
                
                if (y_now - y_last < epsilon and y_now != y_last) {
                    m_awake = false;
                }
                 
                y_last = y_now;
                y_now = position.y;
            }
            else if (position.y < rectangleY1) {
                velocity.y = -velocity.y * energyLoss;
            }
            else if (position.x > rectangleX2 - imageWidthScaled) {
                velocity.x = -velocity.x * energyLoss;
                position.x = rectangleX2 - imageWidthScaled;
            }
            else if (position.x < rectangleX1) {
                velocity.x = -velocity.x * energyLoss;
                position.x = rectangleX1;
            }
            if (m_awake) {
                if (position.y < y_now)
                    y_now = position.y;
            }
            else {
                energyLoss = damper(energyLoss, 0, 0.01F);
            }

            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_rectangle(rectangleX1, rectangleY1, rectangleX2, rectangleY2, al_map_rgb(255, 255, 255), rectangleThickness);
            al_draw_scaled_bitmap(player, 0, 0, imageWidth, imageHeight, position.x, position.y, imageWidthScaled, imageHeightScaled, 0);

            
            text << "gravity: " << gravity;
            al_draw_text(font, al_map_rgb(255, 255, 255), rectangleX2 - 100, rectangleY2, ALLEGRO_ALIGN_LEFT, text.str().c_str());
            text.str("");
            text << "energyLoss: " << energyLoss;
            al_draw_text(font, al_map_rgb(255, 255, 255), rectangleX2 - 100, rectangleY2 + 40, ALLEGRO_ALIGN_LEFT, text.str().c_str());
            text.str("");

            al_flip_display();
        }
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_bitmap(player);
    al_destroy_event_queue(event_queue);

    return 0;
}