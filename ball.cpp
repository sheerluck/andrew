// Dive into C++11

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

// http://stackoverflow.com/questions/20007961/error-running-a-compiled-c-file-uses-opengl-error-inconsistency-detected
#include <pthread.h>
void junk() {
  int i;
  i=pthread_getconcurrency();
};

// g++ -pthread -std=c++11 ball.cpp -o ball -I./include -L./dist/lib/ -lsfml-system -lsfml-graphics -lsfml-window -lsfml-audio
using namespace std;
using namespace sf;

constexpr int winW{800}, winH{600};
constexpr double ballR{10.0}, ballV{3.33};
constexpr double padW{77.7},  padH{17.7},  padV{4.4};

template<typename Shape>
struct Boo {
    Shape shape;
    Vector2f velocity;
    Boo(double mX, double mY, Color color, Vector2f o, Vector2f v)
    : velocity{v} {
        shape.setPosition(mX, mY);
        shape.setFillColor(color);
        shape.setOrigin(o.x, o.y);
    }
    virtual void update() = 0;
    virtual double offX() = 0;
    virtual double offY() = 0;
    double x()    { return shape.getPosition().x; }
    double y()    { return shape.getPosition().y; }
    double left() { return x() - offX(); }
    double rite() { return x() + offX(); }
    double top()  { return y() - offY(); }
    double bot()  { return y() + offY(); }
}; // struct Boo

struct Pad : Boo<RectangleShape> {
    Pad(double mX, double mY)
    : Boo{mX, mY, Color::Blue, {padW / 2,  padH / 2}, {0.0, 0.0} } { shape.setSize({padW,  padH}); } 
    void update() {
        shape.move(velocity);
        velocity.x = 0; // stop
        auto LeftPressed = Keyboard::isKeyPressed(Keyboard::Key::Left);
        auto RitePressed = Keyboard::isKeyPressed(Keyboard::Key::Right);
        if (LeftPressed && left() >    0) velocity.x = -padV;
        if (RitePressed && rite() < winW) velocity.x = padV;
    }
    virtual double offX() { return shape.getSize().x / 2; }
    virtual double offY() { return shape.getSize().y / 2; }
}; // struct Pad

struct Ball : Boo<CircleShape> {
    Ball(double mX, double mY)
    : Boo{mX, mY, Color::Red, {ballR, ballR}, {-ballV, -ballV} }   { shape.setRadius(ballR);       }
    void update() {
        shape.move(velocity);
        if (left() <    0) velocity.x =  ballV;
        if (rite() > winW) velocity.x = -ballV;
        if (top()  <    0) velocity.y =  ballV;
        if (bot()  > winH) velocity.y = -ballV;
    }
    virtual double offX() { return shape.getRadius(); }
    virtual double offY() { return shape.getRadius(); }
}; // struct Ball

template<typename A, typename B>
bool isInter(A& a, B& b) {
    return    a.rite() >= b.left()  
           && a.left() <= b.rite()
           && a.bot()  >= b.top()
           && a.top()  <= b.bot();
} // bool isInter(A& a, B& b)

template<typename A, typename B>
void test(A& p, B& b) {
    if(isInter(p,b)) {
        b.velocity.x = b.x() < p.x() ? -ballV : ballV;
        b.velocity.y = -ballV; // up
    }
}

int main()
{
    auto ball = Ball{winW / 2, winH / 2 };
    auto pad  = Pad {winW / 2, winH - 44};

    //auto win  = RenderWindow{ {winW, winH}, "Z" };  // error: call to implicitly-deleted copy constructor of 'sf::RenderWindow'
    RenderWindow win{ {winW, winH}, "Z" };

    win.setFramerateLimit(60);

    while(true) {
        win.clear(Color::Black);
        if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) break;

        ball.update();
        pad .update();

        test(pad, ball);

        win.draw(ball.shape);
        win.draw(pad .shape);
        win.display();
    }
}

// 00:22:11
