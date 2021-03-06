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

constexpr int    winW{800},   winH{600};
constexpr double ballR{10.0}, ballV{3.33};
constexpr double padW{77.7},  padH{17.7},  padV{4.4};
constexpr double brckW{55.5}, brckH{22.2};
constexpr int    brckX{12},   brckY{6};

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
    : Boo{mX, mY, Color::Blue, {padW / 2, padH / 2}, {0.0, 0.0} }
    { shape.setSize({padW, padH}); } 
    virtual void update() {
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

struct Brck : Boo<RectangleShape> {
    bool destroyed{false};
    Brck(double mX, double mY)
    : Boo{mX, mY, Color::Yellow, {brckW / 2, brckH / 2}, {0.0, 0.01} }
    { shape.setSize({brckW, brckH}); } 
    virtual void update() {
        shape.move(velocity);
    }
    virtual double offX() { return shape.getSize().x / 2; }
    virtual double offY() { return shape.getSize().y / 2; }
}; // struct Brck

struct Ball : Boo<CircleShape> {
    bool nice{true};
    Ball(double mX, double mY)
    : Boo{mX, mY, Color::Red, {ballR, ballR}, {-ballV, -ballV} }
    { shape.setRadius(ballR); }
    virtual void update() {
        shape.move(velocity);
        if (left() <    0) velocity.x =  ballV;
        if (rite() > winW) velocity.x = -ballV;
        if (top()  <    0) velocity.y =  ballV;
        if (bot()  > winH) nice = false;
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
} // void test(A& p, B& b)

template<typename A, typename B>
void test2(A& br, B& b) {
    if(isInter(br,b)) {
        br.destroyed = true;
        auto oL = b .rite() - br.left();
        auto oR = br.rite() - b .left();
        auto oT = b .bot()  - br.top();
        auto oB = br.bot()  - b .top();
        auto fromL = abs(oL) < abs(oR);
        auto fromT = abs(oT) < abs(oB);
        auto minox = fromL ? oL : oR;
        auto minoy = fromT ? oT : oB;
        auto da = abs(minox) < abs(minoy);
        if(da) b.velocity.x = fromL ? -ballV : ballV;
        else   b.velocity.y = fromT ? -ballV : ballV;
    }
} // void test2(A& p, B& b)

void fill_up_bricks_vector(vector<Brck>& br) {
    // ah, use range(), Luke!
    for(int X{0}; X < brckX; X++)
        for(int Y{0}; Y < brckY; Y++) {
            auto x = (X + 1) * (brckW + 3) + 22;
            auto y = (Y + 1) * (brckH + 3);
            br.emplace_back(x, y);
        }
} // void fill_up_bricks_vector(vector<Brck>& x)

int main()
{
    auto ball = Ball{winW / 2, winH / 2 };
    auto pad  = Pad {winW / 2, winH - 44};
    auto brx  = vector<Brck>{};
    fill_up_bricks_vector(brx);

    //auto win  = RenderWindow{ {winW, winH}, "Z" };  // error: call to implicitly-deleted copy constructor of 'sf::RenderWindow'
    RenderWindow win{ {winW, winH}, "Z" };

    win.setFramerateLimit(60);

    while( ! Keyboard::isKeyPressed(Keyboard::Key::Escape)
          && ball.nice ) {

        win.clear(Color::Black);
        win.draw(ball.shape);
        win.draw(pad .shape);
        for(auto& b : brx) win.draw(b.shape);
        win.display();

        ball.update();
        pad .update();
        for(auto& b : brx) b.update();

        test(pad, ball);
        for(auto& b : brx) test2(b, ball);
        brx.erase(remove_if( begin(brx), end(brx),
                             [] (Brck& b){ return b.destroyed; }),
                  end(brx));
    }
}
