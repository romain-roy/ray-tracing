#include <ctime>
#include <fstream>
#include <iostream>

#include "raytracing.h"

int main()
{
    printf("RAY TRACING by Romain Roy\n-------------------------\n");

    Light light;
    Boxs boxs;

    if (!init_scene(light, boxs))
        return 1;

    std::time_t t1 = std::time(0);
    std::tm *start = std::localtime(&t1);
    printf("Start time: %d:%d:%d\n", start->tm_hour, start->tm_min, start->tm_sec);

    printf("Rendering image... 0 %%\r");

    if (render_image(light, boxs))
    {
        std::time_t t2 = std::time(0);
        std::tm *finish = std::localtime(&t2);
        printf("Finish time: %d:%d:%d\n", finish->tm_hour, finish->tm_min, finish->tm_sec);

        std::time_t t3 = t2 - t1;
        std::tm *compute = std::localtime(&t3);
        printf("Compute time: %d:%d:%d\n", (compute->tm_hour - 1), compute->tm_min, compute->tm_sec);
        return 0;
    }

    return 1;
}