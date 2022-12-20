#include <iostream>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <cstdlib>
#include <list>
#include <cmath>
#include <sstream>
#include <string>
using namespace sf;

const int W = 1200;
const int H = 800;

float DEGTORAD = 0.017453f;

class Animation
{
   public:
   float Frame, speed;
   Sprite sprite;
   std::vector<IntRect> frames;

   Animation(){}

   Animation (Texture &t, int x, int y, int w, int h, int count, float Speed)
   {
     Frame = 0;
     speed = Speed;

     for (int i=0;i<count;i++)
      frames.push_back( IntRect(x+i*w, y, w, h)  );

     sprite.setTexture(t);
     sprite.setOrigin(w/2,h/2);
     sprite.setTextureRect(frames[0]);
   }


   void update()
   {
     Frame += speed;
     int n = frames.size();
     if (Frame >= n) Frame -= n;
     if (n>0) sprite.setTextureRect( frames[int(Frame)] );
   }

   bool isEnd()
   {
     return Frame+speed>=frames.size();
   }

};


class Entity
{
   public:
   float x,y,dx,dy,R,angle;
   bool life;
   std::string name;
   Animation anim;

   Entity()
   {
     life=1;
   }

   void settings(Animation &a,int X,int Y,float Angle=0,int radius=1)
   {
     anim = a;
     x=X; y=Y;
     angle = Angle;
     R = radius;
   }

   virtual void update(){};

   void draw(RenderWindow &app)
   {
     anim.sprite.setPosition(x,y);
     anim.sprite.setRotation(angle+90);
     app.draw(anim.sprite);

     CircleShape circle(R);
     circle.setFillColor(Color(255,0,0,170));
     circle.setPosition(x,y);
     circle.setOrigin(R,R);
     //app.draw(circle);
   }

   virtual ~Entity(){};
};


class enemy: public Entity
{
   public:
   enemy()
   {
     dx=rand()%8-4 ;  //
     dy=rand()%8-4;
     name="enemy";
   }

   void update()
   {
     x+=dx;
     y+=dy;

     if (x>W) x=0;  if (x<0) x=W;
     if (y>H) y=0;  if (y<0) y=H;
   }

};


class bullet: public Entity
{
   public:
   bullet()
   {
     name="bullet";
   }

   void  update()
   {
     dx=cos(angle*DEGTORAD)*6;
     dy=sin(angle*DEGTORAD)*6;
     // angle+=rand()%7-3;  /*try this*/
     x+=dx;
     y+=dy;

     if (x>W || x<0 || y>H || y<0) life=0;
   }

};


class player: public Entity
{
   public:
   bool thrust;

   player()
   {
     name="player";
   }

   void update()
   {
     if (thrust)
      { dx+=cos(angle*DEGTORAD)*0.2;
        dy+=sin(angle*DEGTORAD)*0.2; }
     else
      { dx*=0.99;
        dy*=0.99; }

    int maxSpeed=5;
    float speed = sqrt(dx*dx+dy*dy);
    if (speed>maxSpeed)
     { dx *= maxSpeed/speed;
       dy *= maxSpeed/speed; }

    x+=dx;
    y+=dy;

    if (x>W) x=0; if (x<0) x=W;
    if (y>H) y=0; if (y<0) y=H;
   }

};


bool isCollide(Entity *a,Entity *b)   // hit
{
  return (b->x - a->x)*(b->x - a->x)+
         (b->y - a->y)*(b->y - a->y)<
         (a->R + b->R)*(a->R + b->R);
}


int main()
{
    srand(time(0));

    RenderWindow app(VideoMode(W, H), "Kamikaze!");
    app.setFramerateLimit(60);

    Texture t1,t2,t3,t4,t5,t6,t7;
    t1.loadFromFile("images/Ally Aircraft 1.png");
    t2.loadFromFile("images/background1.jpg");
    t3.loadFromFile("images/explosions/type_C.png");
    t4.loadFromFile("images/Japan Aircraft.png");
    t5.loadFromFile("images/fire_red.png");
    t6.loadFromFile("images/Ally Aircraft 2.png");
    t7.loadFromFile("images/explosions/type_B.png");

    t1.setSmooth(true);
    t2.setSmooth(true);

    Sprite background(t2);

    Animation sExplosion(t3, 0,0,256,256, 48, 0.5);
    Animation sPlane(t4, 0,0,60,60, 1, 0);
    Animation sBullet(t5, 0,0,32,64, 16, 0.8);
    Animation sPlayer(t1, 0,0,60,60, 1, 0);
    Animation sPlayer_go(t6, 0,0,60,60, 1, 0);
    Animation sExplosion_ship(t7, 0,0,192,192, 64, 0.5);


    std::list<Entity*> entities;

    for(int i=0;i<3;i++)        //enemy spawns
    {
      enemy *a = new enemy();
      a->settings(sPlane, 0, 0, 90, 1);
      entities.push_back(a);
    }

    player *p = new player();
    p->settings(sPlayer,200,200,0,1);
    entities.push_back(p);

  int score = 0;                                      //score poiint
 
    sf::Font Bodoni;
    Bodoni.loadFromFile("font/Bodoni Bd BT Bold.ttf");
 
    std::ostringstream ssScore;
    ssScore << "Score: " << score;
 
    sf::Text Score;
    Score.setCharacterSize(20);
    Score.setPosition({ 10, 10 });
    Score.setFont(Bodoni);
    Score.setString(ssScore.str());


    /////main loop/////
    while (app.isOpen())
    {
        Event event;
        while (app.pollEvent(event))
        {
            if (event.type == Event::Closed)
                app.close();

            if (event.type == Event::KeyPressed)
             if (event.key.code == Keyboard::Space)
              {
                bullet *b = new bullet();
                b->settings(sBullet,p->x,p->y,p->angle,10);
                entities.push_back(b);
              }
        }

    if (Keyboard::isKeyPressed(Keyboard::D)) p->angle+=3;
    if (Keyboard::isKeyPressed(Keyboard::A))  p->angle-=3;
    if (Keyboard::isKeyPressed(Keyboard::W)) p->thrust=true;
    else p->thrust=false;


    for(auto a:entities)
     for(auto b:entities)
     {
      if (a->name=="enemy" && b->name=="bullet")    //bullet hit
       if ( isCollide(a,b) )
           {
            a->life=false;
            b->life=false;

            Entity *e = new Entity();
            e->settings(sExplosion,a->x,a->y);
            e->name="explosion";
            entities.push_back(e);
            score++;                            
            ssScore.str("");                    // score will add if a&b collide
            ssScore << "Score:  " << score;
            Score.setString(ssScore.str());


           }

      if (a->name=="player" && b->name=="enemy")   //player hit
       if ( isCollide(a,b) )
           {
            b->life=false;

            Entity *e = new Entity();
            e->settings(sExplosion_ship,a->x,a->y);
            e->name="explosion";
            entities.push_back(e);

            score--;                            
            ssScore.str("");                    // score will remove if a&b collide
            ssScore << "Score:  " << score;
            Score.setString(ssScore.str());

            p->settings(sPlayer,W/2,H/2,0,20);
            p->dx=0; p->dy=0;
        }
     }


    if (p->thrust)  p->anim = sPlayer_go;
    else   p->anim = sPlayer;


    for(auto e:entities)
     if (e->name=="explosion")
      if (e->anim.isEnd()) e->life=0;


    if (rand()%50==0)
     {
       enemy*a = new enemy();
       a->settings(sPlane, 0,0, rand()%90, 25);
       entities.push_back(a);
     }

    for(auto i=entities.begin();i!=entities.end();)
    {
      Entity *e = *i;

      e->update();
      e->anim.update();

      if (e->life==false) {i=entities.erase(i); delete e;}
      else i++;
    }

   //////draw//////
   app.draw(background);
   app.draw(Score);
   for(auto i:entities) i->draw(app);
   app.display();
    }

    return 0;
}
