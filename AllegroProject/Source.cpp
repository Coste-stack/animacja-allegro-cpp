#include <sstream>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

#define ScreenWidth 800
#define ScreenHeight 600

// nie u¿ywane ale do zmiany
bool Collision(float x, float y, int width, float ex1, float ey1, float ex2, float ey2) {
    if (x + width < ex2 && x > ex1 && y + width < ey2 && y > ey1) {
        return false;
    }
    return true;
}

int main()
{
    al_init();
    al_init_primitives_addon(); // al_draw_rectangle
    ALLEGRO_DISPLAY* display;

    if (!al_init())
        al_show_native_message_box(NULL, "Error", NULL, "Could not Initialize Allegro", NULL, NULL);

    display = al_create_display(ScreenWidth, ScreenHeight);

    if (!display)
        al_show_native_message_box(NULL, "Error", NULL, "Could not create Allegro Display", NULL, NULL);

    al_set_window_position(display, 100, 100); // gdzie na ekrenie ma sie pojawiæ okno

    // do tekstu
    al_init_font_addon();
    al_init_ttf_addon();
    ALLEGRO_FONT* font = al_load_font("OpenSans-Regular.ttf", 36, NULL);
    std::ostringstream buffer;

    float x = 300, y = 200; // lokalizacja spawnu ludzika
    float x_velocity = 2.0, y_velocity = 7.0;
    float gravity = 0.1, energyLoss = 0.8;

    // pobranie bitmapy
    al_init_image_addon();
    ALLEGRO_BITMAP* player = al_load_bitmap("Bitmapa.png");
    int imageWidth = al_get_bitmap_width(player);
    int imageHeight = al_get_bitmap_height(player);
    float imageScale = 0.3;
    float imageWidthScaled = imageWidth * imageScale;
    float imageHeightScaled = imageHeight * imageScale;

    int rectangleX1 = 200, rectangleX2 = 500;
    int rectangleY1 = 100, rectangleY2 = 400;
    int rectangleThickness = 5;

    const float FPS = 60.0; // szybkoœæ updatowania zmiany sprite-u postaci
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_start_timer(timer);

    bool done = false; // kiedy skoñczyæ program
    while (!done) {
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);

        if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true; // zakoñcz program jak zamknie sie terminal
        }
        else if (events.type == ALLEGRO_EVENT_TIMER) { // ????? coœ z event_queue (mo¿e czêstoœæ odœwie¿ania ekranu)
            x += x_velocity;
            y += y_velocity;
            y_velocity += gravity;

            // kolizja z doln¹ œcian¹
            if (y + imageWidthScaled > rectangleY2) { 
                y_velocity = -y_velocity * energyLoss;
                x_velocity = x_velocity * energyLoss;
                y = rectangleY2 - imageWidthScaled; // aby kulka sie nie bugowa³a przy kolizji
            }
            // kolizja z górn¹ œcian¹
            else if (y < rectangleY1) { 
                y_velocity = -y_velocity * energyLoss;
                x_velocity = x_velocity * energyLoss;
            }
            // kolizja z praw¹ œcian¹
            else if (x > rectangleX2 - imageWidthScaled) {
                x_velocity = -x_velocity * energyLoss;
                x = rectangleX2 - imageWidthScaled;
            } 
            // kolizja z lew¹ œcian¹
            else if (x < rectangleX1) {
                x_velocity = -x_velocity * energyLoss;
                x = rectangleX1;
            }
        }
        buffer << "x_velocity: " << x_velocity;
        al_draw_text(font, al_map_rgb(255, 255, 255), rectangleX2 - 100, rectangleY2, ALLEGRO_ALIGN_LEFT, buffer.str().c_str());
        buffer.str("");
        buffer << "y_velocity: " << y_velocity;
        al_draw_text(font, al_map_rgb(255, 255, 255), rectangleX2 - 100, rectangleY2 + 40, ALLEGRO_ALIGN_LEFT, buffer.str().c_str());
        buffer.str("");

        al_draw_rectangle(rectangleX1, rectangleY1, rectangleX2, rectangleY2, al_map_rgb(255, 255, 255), rectangleThickness);
        
        al_draw_scaled_bitmap(player, 0, 0, imageWidth, imageHeight, x, y, imageWidthScaled, imageHeightScaled, NULL);

        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0)); // czyszczenie okna z poprzednich ludzików
    }
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_bitmap(player);
    al_destroy_event_queue(event_queue);
    return 0;
}