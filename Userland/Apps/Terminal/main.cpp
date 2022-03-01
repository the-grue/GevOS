#include "draw.hpp"
#include <LibC/poll.hpp>
#include <LibC/stdio.hpp>
#include <LibC/stdlib.hpp>
#include <LibC/string.hpp>
#include <LibC/unistd.hpp>
#include <LibDisplay/connection.hpp>

int main(int argc, char** argv)
{
    /* Become TTY master */
    fchown(0, 1, 0);

    canvas_t canvas;
    int events_file = request_display_window(canvas, 700, 500);
    if (events_file == -1)
        return 0;

    canvas_set(canvas.framebuffer, 0x080808, canvas.size);
    int pid = spawn((char*)"shell", 0);

    request_update_window();

    keyboard_event_t keyboard_event;
    char key_input[2];
    int keys_pressed = 0;
    memset(key_input, 0, sizeof(key_input));

    char stdout_buffer[1024];
    bool continuous_stdout = false;
    memset(stdout_buffer, 0, sizeof(stdout_buffer));

    struct pollfd polls[3];
    polls[0].events = POLLIN;
    polls[0].fd = 0;
    polls[1].events = POLLIN;
    polls[1].fd = 1;
    polls[2].events = POLLIN;
    polls[2].fd = events_file;

    while (1) {
        display_event_t event;
        if (receive_event(&event)) {
            if (event.type == DISPLAY_EVENT_KEYBOARD) {
                memcpy(&keyboard_event, &event.keyboard, sizeof(keyboard_event_t));
                if (keyboard_event.key == 27)
                    break;

                if (keyboard_event.key == 10)
                    keys_pressed = -1;

                if ((keyboard_event.key == '\b') && (keys_pressed <= 0))
                    continue;

                if ((keyboard_event.key == 'c') && (keyboard_event.modifier == 2)) {
                    kill(0, 2);
                    draw_text(&canvas, "\n");
                    pid = spawn((char*)"shell", 0);
                    continue;
                }

                keys_pressed += (keyboard_event.key == '\b') ? -1 : 1;
                write(0, &keyboard_event.key, 1);
            }

            if (event.type == DISPLAY_EVENT_RESIZE) {
                canvas = event.canvas;
                clear_text(&canvas);
                request_update_window();

                /* Force new prompt */
                kill(pid, 2);
                pid = spawn((char*)"shell", 0);
            }
        }

        if (read(1, stdout_buffer, sizeof(stdout_buffer))) {
            draw_text(&canvas, stdout_buffer);
            continuous_stdout = true;
        } else if (continuous_stdout) {
            continuous_stdout = false;
            request_update_window();
        }

        if (!continuous_stdout)
            poll(polls, 3);
    }

    request_destroy_window();
    return 0;
}
