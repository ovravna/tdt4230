// Local headers
#include "program.hpp"
#include "utilities/window.hpp"
#include "gamelogic.h"
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/System/Time.hpp>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <utilities/shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utilities/timeutils.h>


void runProgram(GLFWwindow* window, CommandLineOptions options)
{
    // Enable depth (Z) buffer (accept "closest" fragment)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Configure miscellaneous OpenGL settings
    glEnable(GL_CULL_FACE);

	/* glEnable(GL_STENCIL_TEST); */ 
	/* glStencilFunc(GL_NOTEQUAL, 1, 0xFF); */
	/* glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); */ 
	/* glfwSwapInterval(1); //disable vsync and 60 fps cap */
    /* glDisable(GL_CULL_FACE); */


    // Disable built-in dithering
    glDisable(GL_DITHER);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set default colour after clearing the colour buffer
    glClearColor(0.1f, 0.2f, 0.5f, 1.0f);

	initGame(window, options);
	

    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
	    // Clear colour and depth buffers
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        updateFrame(window);
        renderFrame(window);

        // Handle other events
        glfwPollEvents();
        handleKeyboardInput(window);

        // Flip buffers
        glfwSwapBuffers(window);
    }
}


