#include "tty.hpp"
#include "multitasking.hpp"

TTY::TTY()
{
    pipe_stdout = Pipe::create();
    pipe_stdin = Pipe::create();
}

TTY::~TTY()
{
}

int TTY::write_stdout(uint8_t* buffer, uint32_t size)
{
    return Pipe::append(pipe_stdout, buffer, size);
}

int TTY::read_stdout(uint8_t* buffer, uint32_t size)
{
    memset(buffer, 0, size);
    return Pipe::read(pipe_stdout, buffer, size);
}

int TTY::write_stdin(uint8_t* buffer, uint32_t size)
{
    /* FIXME: Properly handle backspace characters */
    bool is_backspace = (buffer[size - 1] == '\b');
    if (read_stdin_size <= 0)
        return 0;

    if ((is_backspace && can_backspace()) || !is_backspace) {
        write_stdout(buffer, size);
        stdin_keypress_size += is_backspace ? -1 : 1;

        if (is_backspace) {
            if (!pipe_stdin->size)
                return 0;
            pipe_stdin->buffer[pipe_stdin->size - 1] = 0;
            pipe_stdin->size--;
        } else {
            return Pipe::append(pipe_stdin, buffer, size);
        }
    }

    return 0;
}

int TTY::read_stdin(uint8_t* buffer, uint32_t size)
{
    memset(buffer, 0, size);
    return Pipe::read(pipe_stdin, buffer, size);
}

bool TTY::should_wake_stdin()
{
    if (pipe_stdin->size == 0)
        return false;
    return ((pipe_stdin->size >= read_stdin_size)
        || (pipe_stdin->buffer[pipe_stdin->size - 1] == 10));
}

int TTY::task_read_stdin(uint8_t* buffer, uint32_t size)
{
    read_stdin_size = size;
    TM->task()->sleep(-SLEEP_WAIT_STDIN);
    TM->yield();
    read_stdin_size = 0;
    stdin_keypress_size = 0;
    return Pipe::read(pipe_stdin, buffer, size);
}
