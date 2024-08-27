#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "Quadtree.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;

Math::MiddleAverageFilter<float,100> fpscounter;

class Ball : public IQuadFitable
{
    sf::Vector2f p;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;

public:
    Ball(int posX, int posY, int dirX, int dirY, int r, int speed)
    {
        p.x = posX;
        p.y = posY;
        dir.x = dirX;
        dir.y = dirY;
        this->r = r;
        this->speed = speed;
    }

    void Draw(sf::RenderWindow& window) const
    {
        sf::CircleShape gball;
        gball.setRadius(r);
        gball.setPosition(p.x, p.y);
        window.draw(gball);
    }

    void Move(float deltaTime)
    {
        float dx = dir.x * speed * deltaTime;
        float dy = dir.y * speed * deltaTime;
        p.x += dx;
        p.y += dy;
    }

    bool IsFitsTheRect(const Rect& rect) const override
    {
        // fix
        return p.x > rect.x && p.x < rect.x + rect.width && p.y > rect.y && p.y < rect.y + rect.height;
    }
};

void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

void draw_quadtree(sf::RenderWindow& window, const Quadtree& quadtree)
{
    for (const auto& childQuad : quadtree.GetChildren())
        draw_quadtree(window, childQuad);

    auto rect = quadtree.GetRect();

    sf::RectangleShape quad;
    quad.setPosition(sf::Vector2f(rect.x, rect.y));
    quad.setSize(sf::Vector2f(rect.width, rect.height));
    quad.setFillColor(sf::Color::Transparent);
    quad.setOutlineColor(sf::Color::Red);
    quad.setOutlineThickness(2);

    window.draw(quad);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    srand(time(NULL));

    std::vector<Ball> balls;
    Quadtree quadtree(Rect(0, 0, WINDOW_X, WINDOW_Y));

    // randomly initialize balls
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    {
        Ball newBall(
            rand() % WINDOW_X,
            rand() % WINDOW_Y,
            (-5 + (rand() % 10)) / 3.,
            (-5 + (rand() % 10)) / 3.,
            5 + rand() % 5,
            30 + rand() % 30
        );
        balls.push_back(newBall);
    }

   // window.setFramerateLimit(60);

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению
        
        quadtree.Clear();

        for (auto& ball : balls)
        {
            quadtree.Insert(&ball);
        }

        for (auto& ball : balls)
        {
            ball.Move(deltaTime);
        }

        window.clear();
        draw_quadtree(window, quadtree);
        for (const auto& ball : balls)
        {
            ball.Draw(window);
        }

		draw_fps(window, fpscounter.getAverage());
		window.display();
    }
    return 0;
}
