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

int RandomNumber(int a, int b) {
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(a, b);
    return distr(gen);
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

class Ball : Square {
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
    ALLEGRO_BITMAP* player;
    int imageWidth, imageHeight;
    float imageScale;
    float imageWidthScaled, imageHeightScaled;

    bool ball_stop = false;
    float y_last, y_now;
public:
    const float epsilon = 2.0;
    float gravity = 0.4, energyLoss = 0.75;
    vec2 ball_position, ball_velocity;

    void Initialize() {
        player = al_load_bitmap("Bitmapa.png");
        imageWidth = al_get_bitmap_width(player);
        imageHeight = al_get_bitmap_height(player);
        imageScale = 0.3;
        imageWidthScaled = imageWidth * imageScale;
        imageHeightScaled = imageHeight * imageScale;

        y_last = square_position.y1;
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
        if (ball_position.y + imageHeightScaled > square_position.y2) {
            ball_velocity.y = -ball_velocity.y * energyLoss;
            ball_position.y = square_position.y2 - imageHeightScaled;

            if (y_now - y_last < epsilon and y_now != y_last) {
                ball_stop = true;
            }

            y_last = y_now;
            y_now = ball_position.y;
        }
        else if (ball_position.y < square_position.y1) {
            ball_velocity.y = -ball_velocity.y * energyLoss;
        }
        else if (ball_position.x > square_position.x2 - imageWidthScaled) {
            ball_velocity.x = -ball_velocity.x * energyLoss;
            ball_position.x = square_position.x2 - imageWidthScaled;
        }
        else if (ball_position.x < square_position.x1) {
            ball_velocity.x = -ball_velocity.x * energyLoss;
            ball_position.x = square_position.x1;
        }

        if (!ball_stop) {
            if (ball_position.y < y_now)
                y_now = ball_position.y;
        }
        else {
            energyLoss = Damper(energyLoss, 0, 0.01);
            if (energyLoss < 0.01 && energyLoss != 0) {
                ball_position.y = square_position.y2 - imageWidthScaled;
                gravity = 0, ball_velocity.y = 0;
            }
        }
    }


};

class BallContainer : Square {
    class FileWriter {
        std::string filepath;
    public:
        FileWriter() = default;

        FileWriter(const std::string& filepath_write) : filepath(filepath_write) {
            std::ofstream file(filepath, std::ios::out | std::ios::trunc);
            file.close();
        }

        void WriteToFile(const std::string& content) {
            std::ofstream file(filepath, std::ios::app);
            try {
                if (!file.is_open()) {
                    throw std::runtime_error("Error could not open file" + filepath + " for writing.");
                }
                file << content << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }
    };
    
    const int offset = 20;
    int ballCounter = 0;

    std::vector<Ball*> balls;
public:
    const std::string filepath_write = "output";
    FileWriter fileWriter{ filepath_write };

    BallContainer(const std::string filepath_write = "output.txt") : filepath_write(filepath_write) {}

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

    void DeleteBall() {
        balls.erase(balls.begin());
    }

    void SimulateBalls() {
        for (const auto& ballPtr : balls) {
            ballPtr->Collision();
            if (ballPtr->energyLoss < 0.005) {
                ballPtr->energyLoss = 0;
                DeleteBall();
            }
        }
    }

    void CreateBall() {
        vec2 ball_position(RandomNumber(square_position.x1 + offset, square_position.x2 - offset), RandomNumber(square_position.y1 + offset, square_position.y2 - offset));
        vec2 ball_velocity(RandomNumber(1, 10), RandomNumber(1, 10));
        AddBall(ball_position, ball_velocity);
        ballCounter += 1;

        std::ostringstream counterStream;
        counterStream << "Ball number: " << ballCounter;
        std::ostringstream positionStream;
        positionStream << "position: " << ball_position.x << ", " << ball_position.y;
        std::ostringstream velocityStream;
        velocityStream << "velocity: " << ball_velocity.x << ", " << ball_velocity.y;

        std::string outputContent = counterStream.str() + "\n-----------------------\n" + positionStream.str() + "\n" + velocityStream.str() + "\n-----------------------\n";
        fileWriter.WriteToFile(outputContent);
    }
};

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
    ALLEGRO_TIMER* timer_ball_spawn = al_create_timer(deltaTime * 100);

    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_timer_event_source(timer_ball_spawn));
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());

    ALLEGRO_FONT* font = al_load_font("OpenSans-Regular.ttf", 36, 0);
    std::ostringstream text;

    vec4 square_position(200, 100, 500, 400);
    int square_thickness = 5;
    Square box(square_position, square_thickness);

    const std::string filepath_output = "output.txt";
    BallContainer ballList(filepath_output);
    ballList.CreateBall();

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
            else if (events.timer.source == timer_ball_spawn) {
                ballList.CreateBall();
            }
            
        }
        else if (events.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Dodaj nową piłkę po lewym kliknięciu myszy
            if (events.mouse.button & 1)
                ballList.CreateBall();
        }
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_timer(timer_ball_spawn);
    al_destroy_event_queue(event_queue);

    return 0;
}