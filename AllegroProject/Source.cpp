#include <iostream>
#include <sstream>
#include <vector>
#include <random>
#include <fstream>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>

#define ScreenWidth 800
#define ScreenHeight 600

const float deltaTime = 1.0 / 60.0;

std::random_device rd;  // Declare random_device globally

struct vec2 {
    float x, y;

    // Default constructor (initialize to zero)
    vec2() : x(0.0), y(0.0) {}

    // Constructor with three arguments
    vec2(float x, float y) : x(x), y(y) {}
};

struct vec4 {
    int x1, x2, y1, y2;

    vec4(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

float Damper(float value, float goal, float factor = 0.1) {
    return (1.0f - factor) * value + factor * goal;
}

class Drawable {
public:
    virtual void Draw() = 0;
    virtual ~Drawable() {}
};

class Square : public Drawable {
public:
    vec4 square_position;
    int square_thickness;
    Square() : square_position(200, 100, 500, 400), square_thickness(5) {}

    Square(vec4 square_position, int square_thickness = 5) : square_position(square_position), square_thickness(square_thickness) {}

    void Draw() override {
        al_draw_rectangle(square_position.x1, square_position.y1, square_position.x2, square_position.y2, al_map_rgb(255, 255, 255), square_thickness);
    }
};

class Ball {
    class FileWriter {
        std::string filename;
    public:
        FileWriter(const std::string& filename) : filename(filename) {}

        void WriteToFile(const std::string& content) {
            std::ofstream file(filename, std::ios::app);
            if (file.is_open()) {
                file << content << std::endl;
                file.close();
            }
            else {
                // Handle file opening error
                std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
            }
        }
    };
    class LineReader {
        std::vector<std::string> lines;
        const std::string& filename;
    public:
        LineReader(const std::string& filename) : filename(filename) {}

        // Function to read lines from a file and store them in a vector
        bool readLinesFromFile() {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error opening file: " << filename << std::endl;
                return false;
            }

            std::string line;
            while (std::getline(file, line)) {
                lines.push_back(line);
            }

            file.close();
            return true;
        }

        // Function to get the vector of lines
        const std::vector<std::string>& getLines() const {
            return lines;
        }
    };

    /*if (lineReader.readLinesFromFile("example.txt")) {
        const std::vector<std::string>& linesFromFile = lineReader.getLines();
        std::cout << "Lines from file:" << std::endl;
        for (const auto& line : linesFromFile) {
            std::cout << line << std::endl;
        }
    }*/

    Square classSquare;

    ALLEGRO_BITMAP* player;
    int imageWidth, imageHeight;
    float imageScale;
    float imageWidthScaled, imageHeightScaled;

    bool ball_stop = false;
    float y_last, y_now;

public:
    const float epsilon = 3.0;
    float gravity = 0.3, energyLoss = 0.8;
    vec2 ball_position, ball_velocity;
    const float eps = 1e-6;

    void Initialize() {
        vec4 square_position(200, 100, 500, 400);
        int square_thickness = 5;
        Square box(square_position, square_thickness);
        classSquare = box;

        player = al_load_bitmap("Bitmapa.png");
        imageWidth = al_get_bitmap_width(player);
        imageHeight = al_get_bitmap_height(player);
        imageScale = 0.3;
        imageWidthScaled = imageWidth * imageScale;
        imageHeightScaled = imageHeight * imageScale;

        y_last = classSquare.square_position.y1;
    }

    Ball() : ball_position(300.0, 200.0), ball_velocity(2.0, 7.0), y_now(ball_position.y) {
        Initialize();
    }

    Ball(vec2 ball_position, vec2 ball_velocity) : ball_position(ball_position), ball_velocity(ball_velocity), y_now(ball_position.y) {
        Initialize();
    }

    ~Ball() {
        al_destroy_bitmap(player);
    }

    void Draw() {
        //al_draw_tinted_scaled_bitmap(player, al_map_rgb(random_number(0, 255), random_number(0, 255), random_number(0, 255)), 0, 0, imageWidth, imageHeight, position.x, position.y, imageWidthScaled, imageHeightScaled, 0);
        al_draw_scaled_bitmap(player, 0, 0, imageWidth, imageHeight, ball_position.x, ball_position.y, imageWidthScaled, imageHeightScaled, 0);
    }

    void Collision() {
        ball_position.x += ball_velocity.x * energyLoss;
        ball_position.y += ball_velocity.y * energyLoss;
        ball_velocity.y += gravity;

        // kolizja
        if (ball_position.y + imageHeightScaled > classSquare.square_position.y2) {
            ball_velocity.y = -ball_velocity.y * energyLoss;
            ball_position.y = classSquare.square_position.y2 - imageHeightScaled;

            if (y_now - y_last < epsilon and y_now != y_last) {
                ball_stop = true;
            }

            y_last = y_now;
            y_now = ball_position.y;
        }
        else if (ball_position.y < classSquare.square_position.y1) {
            ball_velocity.y = -ball_velocity.y * energyLoss;
        }
        else if (ball_position.x > classSquare.square_position.x2 - imageWidthScaled) {
            ball_velocity.x = -ball_velocity.x * energyLoss;
            ball_position.x = classSquare.square_position.x2 - imageWidthScaled;
        }
        else if (ball_position.x < classSquare.square_position.x1) {
            ball_velocity.x = -ball_velocity.x * energyLoss;
            ball_position.x = classSquare.square_position.x1;
        }

        if (!ball_stop) {
            if (ball_position.y < y_now)
                y_now = ball_position.y;
        }
        else {
            energyLoss = Damper(energyLoss, 0, 0.01);
            std::cout << energyLoss << std::endl;
            if (energyLoss < 0.01) {
                ball_position.y = classSquare.square_position.y2 - imageWidthScaled;
                energyLoss = 0, gravity = 0, ball_velocity.y = 0;
            }
        }
    }

    void Display() {
        std::cout << "-----------------------" << std::endl;
        std::cout << "position: " << ball_position.x << ", " << ball_position.y << std::endl;
        std::cout << "velocity: " << ball_velocity.x << ", " << ball_velocity.y << std::endl;
        std::cout << "-----------------------" << std::endl;
    }
};

class BallContainer {
public:
    std::vector<Ball*> balls;

    BallContainer() {}

    ~BallContainer() {
        for (const auto& ballPtr : balls) {
            delete ballPtr;
        }
        balls.clear();
    }

    void AddBall(vec2 ball_position, vec2 ball_velocity) {
        Ball* ball = new Ball(ball_position, ball_velocity);
        balls.push_back(ball);
    }

    void DrawBalls() const {
        for (const auto& ballPtr : balls) {
            ballPtr->Draw();
        }
    }

    void SimulateBalls() const {
        for (const auto& ballPtr : balls) {
            ballPtr->Collision();
        }
    }
};

int RandomNumber(int a, int b) {
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(a, b);
    return distr(gen);
}

void CreateBall(BallContainer &ballList, const vec4& square) {
    const int offset = 20;
    vec2 position(RandomNumber(square.x1 + offset, square.x2 - offset), RandomNumber(square.y1 + offset, square.y2 - offset));
    vec2 velocity(RandomNumber(1, 10), RandomNumber(1, 10));
    ballList.AddBall(position, velocity);
}

int main() {
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();

    ALLEGRO_DISPLAY* display = al_create_display(ScreenWidth, ScreenHeight);
    ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(deltaTime);
    ALLEGRO_TIMER* timer_ball_spawn = al_create_timer(deltaTime * 250);

    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_timer_event_source(timer_ball_spawn));
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());

    ALLEGRO_FONT* font = al_load_font("OpenSans-Regular.ttf", 36, 0);
    std::ostringstream text;

    vec4 square_position(200, 100, 500, 400);
    int square_thickness = 5;
    Square box(square_position, square_thickness);

    vec2 position(300.0, 200.0);
    vec2 velocity(2.0, 7.0);
    vec2 pos(350.0, 250.0);
    vec2 vel(0.0, 2.0);

    BallContainer ballList;
    ballList.AddBall(position, velocity);
    //ballList.AddBall(pos, vel);
    //ballList.AddBall(pos, velocity);

    al_start_timer(timer);
    al_start_timer(timer_ball_spawn);
    bool done = false;
    while (!done) {
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);

        if (events.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }
        else if (events.type == ALLEGRO_EVENT_TIMER) {
            if (events.timer.source == timer) {
                ballList.DrawBalls();
                ballList.SimulateBalls();

                box.Draw();
                al_flip_display();
                al_clear_to_color(al_map_rgb(0, 0, 0));
            }
            //else if (events.timer.source == timer_ball_spawn) {
            //    std::cout << 1;
            //    //CreateBall(ballList, square_position);
            //    for (auto element : ballList.balls) {
            //        element->Display();
            //    }
            //}
            
        }
        //else if (events.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        //    // Dodaj nową piłkę po lewym kliknięciu myszy
        //    if (events.mouse.button & 1)
        //        CreateBall(ballList, square_position);
        //}
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_timer(timer_ball_spawn);
    al_destroy_event_queue(event_queue);

    return 0;
}