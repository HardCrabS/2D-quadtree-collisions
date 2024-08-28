#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"
#include "Quadtree.h"
#include "utils.h"

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;

Math::MiddleAverageFilter<float,100> fpscounter;

class Ball : public IQuadFitable
{
public:
    sf::Vector2f p;
    float r = 0;
    sf::Vector2f dir;
    float speed = 0;
    float mass = 0;

    Ball(int posX, int posY, int dirX, int dirY, int r, int speed)
    {
        p.x = posX;
        p.y = posY;
        dir.x = dirX;
        dir.y = dirY;
        this->r = r;
        this->speed = speed;
        mass = r;
    }

    sf::Vector2f GetSpeedVec() { return dir * speed; }

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
        return p.x - r > rect.x && p.x + r < rect.x + rect.width && p.y - r > rect.y && p.y + r < rect.y + rect.height;
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

void draw_quadtree(sf::RenderWindow& window, Quadtree& quadtree)
{
    for (auto& childQuad : quadtree.GetChildren())
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

void process_balls_collision(Ball* c1, Ball* c2)
{
    float distanceBetweenCenters = Math::length(c1->p - c2->p);
    float maxDistancePossible = c1->r + c2->r;

    if (distanceBetweenCenters <= maxDistancePossible)
    {
        // correct positions to avoid sticking
        float error = maxDistancePossible - distanceBetweenCenters;
        sf::Vector2f c1Toc2Dir = Math::normalized(c2->p - c1->p);
        c1->p -= c1Toc2Dir * (error * 0.5f);
        c2->p += c1Toc2Dir * (error * 0.5f);

        sf::Vector2f v1 = ((c1->mass - c2->mass) * c1->GetSpeedVec() + 2 * c2->mass * c2->GetSpeedVec()) / (c1->mass + c2->mass);
        sf::Vector2f v2 = ((c2->mass - c1->mass) * c2->GetSpeedVec() + 2 * c1->mass * c1->GetSpeedVec()) / (c1->mass + c2->mass);

        c1->dir = Math::normalized(v1);
        c1->speed = Math::length(v1);

        c2->dir = Math::normalized(v2);
        c2->speed = Math::length(v2);
    }
}

void process_collisions_quadtree(Quadtree& quadtree)
{
    for (auto& subtree : quadtree.GetChildren())
        process_collisions_quadtree(subtree);

    // check intersection of every ball with every ball within quad
    for (auto it1 = quadtree.GetObjects().begin(); it1 != quadtree.GetObjects().end(); it1++)
    {
        Ball* c1 = static_cast<Ball*>(*it1);

        // screen border collision
        if (c1->p.x - c1->r <= 0 || c1->p.x + c1->r >= WINDOW_X)
            c1->dir = sf::Vector2f(-c1->dir.x, c1->dir.y);
        if (c1->p.y - c1->r <= 0 || c1->p.y + c1->r >= WINDOW_Y)
            c1->dir = sf::Vector2f(c1->dir.x, -c1->dir.y);

        for (auto it2 = it1 + 1; it2 != quadtree.GetObjects().end(); it2++)
        {
            Ball* c2 = static_cast<Ball*>(*it2);
            
            process_balls_collision(c1, c2);
        }

        // if tree is splited then it only contains objects that couldn't fully fit in any subquad
        // therefore check collision with objects from all subquads
        for (auto& subtree : quadtree.GetChildren())
        {
            for (auto it2 = subtree.GetObjects().begin(); it2 != subtree.GetObjects().end(); it2++)
            {
                Ball* c2 = static_cast<Ball*>(*it2);

                process_balls_collision(c1, c2);
            }
        }
    }
}

// O(n^2) just for comparison
void process_collisions_quadratic(std::vector<Ball>& balls)
{
    for (auto c1 = balls.begin(); c1 != balls.end(); c1++)
    {
        if (c1->p.x - c1->r <= 0 || c1->p.x + c1->r >= WINDOW_X)
            c1->dir = sf::Vector2f(-c1->dir.x, c1->dir.y);
        if (c1->p.y - c1->r <= 0 || c1->p.y + c1->r >= WINDOW_Y)
            c1->dir = sf::Vector2f(c1->dir.x, -c1->dir.y);

        for (auto c2 = c1 + 1; c2 != balls.end(); c2++)
        {
            process_balls_collision(&(*c1), &(*c2));
        }
    }
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

        process_collisions_quadtree(quadtree);
        //process_collisions_quadratic(balls);  // just for comparison

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
