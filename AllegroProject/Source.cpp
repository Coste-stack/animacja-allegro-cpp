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

    vec2() : x(0.0), y(0.0) {}
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

class Ball : public Square {
    ALLEGRO_BITMAP* player;
    int imageWidth, imageHeight;
    float imageScale;
    float imageWidthScaled, imageHeightScaled;

    bool ball_stop = false;
    float y_last, y_now;

    int ballColorRed, ballColorGreen, ballColorBlue;

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

    Ball() : ball_position(300.0, 200.0), ball_velocity(2.0, 7.0), y_now(ball_position.y), ballColorRed(255), ballColorGreen(255), ballColorBlue(255) {
        Initialize();
    }

    Ball(vec2 ball_position, vec2 ball_velocity) : ball_position(ball_position), ball_velocity(ball_velocity), y_now(ball_position.y), ballColorRed(255), ballColorGreen(255), ballColorBlue(255) {
        Initialize();
    }

    ~Ball() {
        al_destroy_bitmap(player);
    }

    void Draw() {
        al_draw_tinted_scaled_bitmap(player, al_map_rgb(ballColorRed, ballColorGreen, ballColorBlue),
            0, 0, imageWidth, imageHeight,
            ball_position.x, ball_position.y, imageWidthScaled, imageHeightScaled, 0);
    }

    void SetBallColor(int red, int green, int blue) {
        ballColorRed = red;
        ballColorGreen = green;
        ballColorBlue = blue;
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

class BallContainer : public Square {
public:
    int ballCounter = 0;
    int maxBallNumber = 20;
private:
    class FileReader {
    private:
        std::ifstream file;

    public:
        FileReader(const std::string& filepath_read) : file(filepath_read) {
            if (!file.is_open()) {
                std::cerr << "Error could not open file " << filepath_read << std::endl;
            }
        }

        bool hasNextLine() const {
            return file.is_open() && !file.eof();
        }

        std::string getNextLine() {
            std::string line;
            std::getline(file, line);
            return line;
        }
    };

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

    struct AllBallVariables {
        float pos_x, pos_y, vel_x, vel_y;
        int red, green, blue;

        AllBallVariables(float pos_x, float pos_y, float vel_x, float vel_y, int red, int green, int blue) : pos_x(pos_x), pos_y(pos_y), vel_x(vel_x), vel_y(vel_y), red(red), green(green), blue(blue) {}
    };
    
    std::vector<AllBallVariables> ReadFile(const std::string filepath_read = "input.txt") {
        std::vector<AllBallVariables> ballsCreationQueue;
        FileReader fileReader(filepath_read);
        std::string line;

        if (fileReader.hasNextLine()) {
            line = fileReader.getNextLine();
            std::istringstream numberStream(line);
            numberStream.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // Ignore "Max Balls: "
            numberStream >> maxBallNumber;
            line = fileReader.getNextLine(); // usunąć "------------"
        }

        while (fileReader.hasNextLine()) {
            line = fileReader.getNextLine();

            int ballNumber;
            std::istringstream iss(line);
            iss.ignore(std::numeric_limits<std::streamsize>::max(), ' '); // Ignore "Ball number: "
            iss >> ballNumber;

            float pos_x, pos_y, vel_x, vel_y;
            // Read position line
            line = fileReader.getNextLine();
            std::istringstream positionStream(line);
            positionStream.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            positionStream >> pos_x;
            positionStream.ignore();
            positionStream >> pos_y;

            // Read velocity line
            line = fileReader.getNextLine();
            std::istringstream velocityStream(line);
            velocityStream.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            velocityStream >> vel_x;
            velocityStream.ignore();
            velocityStream >> vel_y;

            int red, green, blue;
            // Read color line
            line = fileReader.getNextLine();
            std::istringstream colorStream(line);
            colorStream.ignore(std::numeric_limits<std::streamsize>::max(), ':');
            colorStream >> red;
            colorStream.ignore();
            colorStream >> green;
            colorStream.ignore();
            colorStream >> blue;
            line = fileReader.getNextLine(); // usunąć "------------"

            AllBallVariables variables(pos_x, pos_y, vel_x, vel_y, red, green, blue);
            ballsCreationQueue.push_back(variables);

        }
        return ballsCreationQueue;
    }

    const int offset = 20;

    std::vector<Ball*> balls;
    std::vector<AllBallVariables> ballsCreationQueue;
public:
    const std::string filepath_write = "output.txt";
    FileWriter fileWriter{ filepath_write };

    const std::string filepath_read = "input.txt";
    FileReader fileReader{ filepath_read };

    BallContainer(const std::string filepath_read = "input.txt", const std::string filepath_write = "output.txt") : filepath_read(filepath_read), filepath_write(filepath_write) {
        ballsCreationQueue = ReadFile(filepath_read);
    }

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

    void FileWrite(vec2 ball_position, vec2 ball_velocity, int red, int green, int blue) {
        // Zapisz informacje o piłce do pliku
        std::ostringstream counterStream;
        counterStream << "Ball number: " << ballCounter;
        std::ostringstream positionStream;
        positionStream << "position: " << ball_position.x << ", " << ball_position.y;
        std::ostringstream velocityStream;
        velocityStream << "velocity: " << ball_velocity.x << ", " << ball_velocity.y;
        std::ostringstream colorStream;
        colorStream << "color: " << red << ", " << green << ", " << blue;

        std::string outputContent = counterStream.str() + "\n" +
            positionStream.str() + "\n" +
            velocityStream.str() + "\n" +
            colorStream.str() + "\n-----------------------";
        fileWriter.WriteToFile(outputContent);
    }

    int red, green, blue;
    void CreateColoredBall() {
        if (ballsCreationQueue.size() != 1) {
            const auto& ballPtr = ballsCreationQueue[ballCounter];

            red = ballPtr.red;
            green = ballPtr.green;
            blue = ballPtr.blue;

            vec2 ball_position(ballPtr.pos_x, ballPtr.pos_y);
            vec2 ball_velocity(ballPtr.vel_x, ballPtr.vel_y);

            AddBall(ball_position, ball_velocity);

            balls.back()->SetBallColor(red, green, blue);
            ballCounter += 1;
            FileWrite(ball_position, ball_velocity, red, green, blue);

            ballsCreationQueue.erase(ballsCreationQueue.begin());
        }
        else {
            // Losuj kolory RGB
            red = RandomNumber(0, 255);
            green = RandomNumber(0, 255);
            blue = RandomNumber(0, 255);

            // Utwórz nową piłkę z losowym kolorem
            vec2 ball_position(RandomNumber(square_position.x1 + offset, square_position.x2 - offset),
                RandomNumber(square_position.y1 + offset, square_position.y2 - offset));
            vec2 ball_velocity(RandomNumber(1, 10), RandomNumber(1, 10));
            AddBall(ball_position, ball_velocity);

            // Ustaw kolor piłki
            balls.back()->SetBallColor(red, green, blue);
            ballCounter += 1;
            FileWrite(ball_position, ball_velocity, red, green, blue);
        }
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

    const std::string filepath_read = "input.txt";
    const std::string filepath_output = "output.txt";
    BallContainer ballList(filepath_read, filepath_output);

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
                ballList.CreateColoredBall();
            }

        }
        else if (events.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Dodaj nową piłkę po lewym kliknięciu myszy
            if (events.mouse.button & 1)
                ballList.CreateColoredBall();
        }
    }

    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_timer(timer);
    al_destroy_timer(timer_ball_spawn);
    al_destroy_event_queue(event_queue);

    return 0;
}
