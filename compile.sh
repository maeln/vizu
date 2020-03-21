#!/bin/bash
gcc main.c -Ilibs/glfw/include -Llibs/glfw -lglfw -Llibs -lnanovg -framework OpenGL -lm 