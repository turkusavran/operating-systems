#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>

const char *sysname = "seashell";
char *getFilePath(char *cmd);
void changeColor(char *color, char *word);
char *readFile(char *document);
void read_shortdir_file();
void insert_shortdir(char *name, char *path);
char *find_shortdir_path(char *name);
void delete_shortdir(char *name);
void print_shortdir();
void update_shortdir_file();

enum return_codes
{
    SUCCESS = 0,
    EXIT = 1,
    UNKNOWN = 2,
};

struct command_t
{
    char *name;
    bool background;
    bool auto_complete;
    int arg_count;
    char **args;
    char *redirects[3];     // in/out redirection
    struct command_t *next; // for piping
};

struct element
{
    char name[50];
    char path[100];
} element;

struct list
{
    struct element *element_list[100];
    int element_count;
} list;

struct list *list_shortdir;
/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command)
{
    int i = 0;
    printf("Command: <%s>\n", command->name);
    printf("\tIs Background: %s\n", command->background ? "yes" : "no");
    printf("\tNeeds Auto-complete: %s\n", command->auto_complete ? "yes" : "no");
    printf("\tRedirects:\n");
    for (i = 0; i < 3; i++)
        printf("\t\t%d: %s\n", i, command->redirects[i] ? command->redirects[i] : "N/A");
    printf("\tArguments (%d):\n", command->arg_count);
    for (i = 0; i < command->arg_count; ++i)
        printf("\t\tArg %d: %s\n", i, command->args[i]);
    if (command->next)
    {
        printf("\tPiped to:\n");
        print_command(command->next);
    }
}
/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command)
{
    if (command->arg_count)
    {
        for (int i = 0; i < command->arg_count; ++i)
            free(command->args[i]);
        free(command->args);
    }
    for (int i = 0; i < 3; ++i)
        if (command->redirects[i])
            free(command->redirects[i]);
    if (command->next)
    {
        free_command(command->next);
        command->next = NULL;
    }
    free(command->name);
    free(command);
    return 0;
}
/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt()
{
    char cwd[1024], hostname[1024];
    gethostname(hostname, sizeof(hostname));
    getcwd(cwd, sizeof(cwd));
    printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
    return 0;
}
/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command)
{
    const char *splitters = " \t"; // split at whitespace
    int index, len;
    len = strlen(buf);
    while (len > 0 && strchr(splitters, buf[0]) != NULL) // trim left whitespace
    {
        buf++;
        len--;
    }
    while (len > 0 && strchr(splitters, buf[len - 1]) != NULL)
        buf[--len] = 0; // trim right whitespace

    if (len > 0 && buf[len - 1] == '?') // auto-complete
        command->auto_complete = true;
    if (len > 0 && buf[len - 1] == '&') // background
        command->background = true;

    char *pch = strtok(buf, splitters);
    command->name = (char *)malloc(strlen(pch) + 1);
    if (pch == NULL)
        command->name[0] = 0;
    else
        strcpy(command->name, pch);

    command->args = (char **)malloc(sizeof(char *));

    int redirect_index;
    int arg_index = 0;
    char temp_buf[1024], *arg;
    while (1)
    {
        // tokenize input on splitters
        pch = strtok(NULL, splitters);
        if (!pch)
            break;
        arg = temp_buf;
        strcpy(arg, pch);
        len = strlen(arg);

        if (len == 0)
            continue;                                        // empty arg, go for next
        while (len > 0 && strchr(splitters, arg[0]) != NULL) // trim left whitespace
        {
            arg++;
            len--;
        }
        while (len > 0 && strchr(splitters, arg[len - 1]) != NULL)
            arg[--len] = 0; // trim right whitespace
        if (len == 0)
            continue; // empty arg, go for next

        // piping to another command
        if (strcmp(arg, "|") == 0)
        {
            struct command_t *c = malloc(sizeof(struct command_t));
            int l = strlen(pch);
            pch[l] = splitters[0]; // restore strtok termination
            index = 1;
            while (pch[index] == ' ' || pch[index] == '\t')
                index++; // skip whitespaces

            parse_command(pch + index, c);
            pch[l] = 0; // put back strtok termination
            command->next = c;
            continue;
        }

        // background process
        if (strcmp(arg, "&") == 0)
            continue; // handled before

        // handle input redirection
        redirect_index = -1;
        if (arg[0] == '<')
            redirect_index = 0;
        if (arg[0] == '>')
        {
            if (len > 1 && arg[1] == '>')
            {
                redirect_index = 2;
                arg++;
                len--;
            }
            else
                redirect_index = 1;
        }
        if (redirect_index != -1)
        {
            command->redirects[redirect_index] = malloc(len);
            strcpy(command->redirects[redirect_index], arg + 1);
            continue;
        }

        // normal arguments
        if (len > 2 && ((arg[0] == '"' && arg[len - 1] == '"') || (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
        {
            arg[--len] = 0;
            arg++;
        }
        command->args = (char **)realloc(command->args, sizeof(char *) * (arg_index + 1));
        command->args[arg_index] = (char *)malloc(len + 1);
        strcpy(command->args[arg_index++], arg);
    }
    command->arg_count = arg_index;
    return 0;
}
void prompt_backspace()
{
    putchar(8);   // go back 1
    putchar(' '); // write empty over
    putchar(8);   // go back 1 again
}
/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command)
{
    int index = 0;
    char c;
    char buf[4096];
    static char oldbuf[4096];

    // tcgetattr gets the parameters of the current terminal
    // STDIN_FILENO will tell tcgetattr that it should write the settings
    // of stdin to oldt
    static struct termios backup_termios, new_termios;
    tcgetattr(STDIN_FILENO, &backup_termios);
    new_termios = backup_termios;
    // ICANON normally takes care that one line at a time will be processed
    // that means it will return if it sees a "\n" or an EOF or an EOL
    new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
    // Those new settings will be set to STDIN
    // TCSANOW tells tcsetattr to change attributes immediately.
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    //FIXME: backspace is applied before printing chars
    show_prompt();
    int multicode_state = 0;
    buf[0] = 0;
    while (1)
    {
        c = getchar();
        // printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

        if (c == 9) // handle tab
        {
            buf[index++] = '?'; // autocomplete
            break;
        }

        if (c == 127) // handle backspace
        {
            if (index > 0)
            {
                prompt_backspace();
                index--;
            }
            continue;
        }
        if (c == 27 && multicode_state == 0) // handle multi-code keys
        {
            multicode_state = 1;
            continue;
        }
        if (c == 91 && multicode_state == 1)
        {
            multicode_state = 2;
            continue;
        }
        if (c == 65 && multicode_state == 2) // up arrow
        {
            int i;
            while (index > 0)
            {
                prompt_backspace();
                index--;
            }
            for (i = 0; oldbuf[i]; ++i)
            {
                putchar(oldbuf[i]);
                buf[i] = oldbuf[i];
            }
            index = i;
            continue;
        }
        else
            multicode_state = 0;

        putchar(c); // echo the character
        buf[index++] = c;
        if (index >= sizeof(buf) - 1)
            break;
        if (c == '\n') // enter key
            break;
        if (c == 4) // Ctrl+D
            return EXIT;
    }
    if (index > 0 && buf[index - 1] == '\n') // trim newline from the end
        index--;
    buf[index++] = 0; // null terminate string

    strcpy(oldbuf, buf);

    parse_command(buf, command);

    // print_command(command); // DEBUG: uncomment for debugging

    // restore the old settings
    tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
    return SUCCESS;
}
int process_command(struct command_t *command);
int main()
{
    read_shortdir_file();
    while (1)
    {
        struct command_t *command = malloc(sizeof(struct command_t));
        memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

        int code;
        code = prompt(command);
        if (code == EXIT)
            break;

        code = process_command(command);
        if (code == EXIT)
            break;

        free_command(command);
    }

    printf("\n");
    return 0;
}
int command_shortdir(struct command_t *command);

int process_command(struct command_t *command)
{
    int r;
    if (strcmp(command->name, "") == 0)
        return SUCCESS;

    else if (strcmp(command->name, "exit") == 0)
        return EXIT;

    else if (strcmp(command->name, "cd") == 0)
    {
        if (command->arg_count > 0)
        {
            r = chdir(command->args[0]);
            if (r == -1)
                printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
            return SUCCESS;
        }
    }
    else if (strcmp(command->name, "shortdir") == 0 && command->arg_count > 0)
        return command_shortdir(command);
    else if (strcmp(command->name, "highlight") == 0 && command->arg_count == 3) //Part3
    {
        char *word = command->args[0];
        char *color = command->args[1];
        char *filename = command->args[2];

        char *mybuff = readFile(filename);

        char *buffer = strtok(mybuff, " ");

        while (buffer != NULL)
        {
            char tempbuffer[100];
            strcpy(tempbuffer, buffer);
            size_t len = strlen(tempbuffer);
            char *lower = calloc(len + 1, sizeof(char));
            for (size_t i = 0; i < len; ++i)
            {
                lower[i] = tolower((char)tempbuffer[i]);
            }

            if (strcmp(lower, word) == 0)
            {
                changeColor(color, buffer);
            }
            /*
            if (strcmp(buffer, word) == 0)
            {
                changeColor(color, buffer);
            }
            */
            else
            {
                printf("%s ", buffer);
            }
            free(lower);
            buffer = strtok(NULL, " ");
        }
        return SUCCESS;
    }

    // Part4

    else if (strcmp(command->name, "goodMorning") == 0)
    {
        char *hourname = command->args[0];
        char *buffer = strtok(hourname, ".");
        char *hr = buffer;
        char *min = strtok(NULL, ".");

        char cmd[222] = " ";
        strcat(cmd, min);
        strcat(cmd, " ");

        strcat(cmd, hr);
        strcat(cmd, " ");

        time_t timer = time(NULL);

        struct tm t;
        t = *localtime(&timer);

        char day[22] = "";
        sprintf(day, "%d", t.tm_mday);
        char month[22] = "";
        sprintf(month, "%d", t.tm_mon);
        char dayweek[22] = "";
        sprintf(dayweek, "%d", t.tm_wday);

        strcat(cmd, day);
        strcat(cmd, " ");
        strcat(cmd, month);
        strcat(cmd, " ");
        strcat(cmd, dayweek);
        strcat(cmd, " ");
        strcat(cmd, "> temp.txt");
        strcat(cmd, " ");

        //strcat("crontab -e ", cmd);
        printf("%s \n", cmd);

        //remove("temp.txt");

        return SUCCESS;
    }

    // Part5
    else if (strcmp(command->name, "kdiff") == 0)
    {
        char *fileOne = command->args[1];
        char *fileTwo = command->args[2];

        int count_lines = 0;
        FILE *fptrOne;
        int iOne = 0;
        fptrOne = fopen(fileOne, "r");
        char chr = getc(fptrOne);

        while (chr != EOF)
        {
            //Count whenever new line is encountered
            if (chr == '\n')
            {
                count_lines = count_lines + 1;
            }
            //take next character from file.
            chr = getc(fptrOne);
        }
        fclose(fptrOne);

        fptrOne = fopen(fileOne, "r");
        char linesOne[count_lines][222];

        while (fgets(linesOne[iOne], 222, fptrOne))
        {
            linesOne[iOne][strlen(linesOne[iOne]) - 1] = '\0';
            iOne++;
        }

        char linesTwo[count_lines][222];
        FILE *fptrTwo;
        int iTwo = 0;
        fptrTwo = fopen(fileTwo, "r");
        while (fgets(linesTwo[iTwo], 222, fptrTwo))
        {
            linesTwo[iTwo][strlen(linesTwo[iTwo]) - 1] = '\0';
            iTwo++;
        }
        int counter = 0;
        if (strcmp(command->args[0], "-a") == 0)
        {
            for (int i = 0; i < count_lines; i++)
            {
                if (strcmp(linesOne[i], linesTwo[i]) != 0)
                {
                    printf("%s:Line %i: %s \n", fileOne, i, linesOne[i]);
                    printf("%s:Line %i: %s \n", fileTwo, i, linesTwo[i]);
                    counter++;
                }
            }
            if (counter > 0)
            {
                printf("%i different lines found \n", counter);
            }
            else
            {
                printf("2 The two text files are identical \n");
            }
        }

        if (strcmp(command->args[0], "-b") == 0)
        {
        }
        fclose(fptrOne);
        fclose(fptrTwo);

        return SUCCESS;
    }
    // Part6

    else if (strcmp(command->name, "whatTime?") == 0)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("Time is %s", asctime(localtime(&tv.tv_sec)));
    }

    pid_t pid = fork();
    if (pid == 0) // child
    {
        // increase args size by 2
        command->args = (char **)realloc(
            command->args, sizeof(char *) * (command->arg_count += 2));

        // shift everything forward by 1
        for (int i = command->arg_count - 2; i > 0; --i)
            command->args[i] = command->args[i - 1];

        // set args[0] as a copy of name
        command->args[0] = strdup(command->name);
        // set args[arg_count-1] (last) to NULL
        command->args[command->arg_count - 1] = NULL;

        // TODO: do your own exec with path resolving using execv()
        char *path;
        path = getFilePath(command->name);
        execv(path, command->args);

        exit(0);
    }
    else
    {
        if (!command->background)
            wait(0); // wait for child process to finish
        return SUCCESS;
    }

    printf("-%s: %s: command not found\n", sysname, command->name);
    return UNKNOWN;
}

// COMMAND FUNCTIONS
int command_shortdir(struct command_t *command)
{
    if (strcmp(command->args[0], "set") == 0)
    {
        char path[100];
        char *shortdirName = command->args[1];
        getcwd(path, sizeof(path));
        insert_shortdir(shortdirName, path);
    }
    else if (strcmp(command->args[0], "jump") == 0)
    {
        char *shortdirName = command->args[1];
        char *path = find_shortdir_path(shortdirName);
        if (path != NULL)
            chdir(path);
        else
            printf("Path not found.\n");
    }
    else if (strcmp(command->args[0], "del") == 0)
    {
        delete_shortdir(command->args[1]);
    }
    else if (strcmp(command->args[0], "clear") == 0)
    {
        list_shortdir->element_count = 0;
    }
    else if (strcmp(command->args[0], "list") == 0)
    {
        print_shortdir();
    }

    update_shortdir_file();
    return SUCCESS;
}

// HELPER FUNCTIONS
// Get the file path
char *getFilePath(char *cmd)
{
    char innerCommand[100] = "which ";
    strcat(innerCommand, cmd);
    strcat(innerCommand, " > temp.txt");
    system(innerCommand);

    FILE *fp;
    fp = fopen("temp.txt", "r");

    char *buff = (char *)malloc(250 * sizeof(char));
    fscanf(fp, "%s", buff);
    fclose(fp);
    remove("temp.txt");
    return buff;
}

// Read shortdir file and save to list shortdir
void read_shortdir_file()
{
    list_shortdir = malloc(sizeof(list));
    list_shortdir->element_count = 0;
    char name[50];
    char path[100];
    FILE *fp;
    fp = fopen("./shortdir.txt", "r");

    while (fscanf(fp, "%s %s", name, path) != EOF)
    {
        insert_shortdir(name, path);
        list_shortdir->element_count++;
    }
    fclose(fp);
}

// Insert or update shortdir - set
void insert_shortdir(char *name, char *path)
{
    struct element *element_shrt = malloc(sizeof(element));
    strcpy(element_shrt->name, name);
    strcpy(element_shrt->path, path);

    int i;
    for (i = 0; i < list_shortdir->element_count; i++)
    {
        // Update if already exists
        if (strcmp(list_shortdir->element_list[i]->name, name) == 0)
        {
            strcpy(list_shortdir->element_list[i]->path, path);
            printf("%s is set as an alias for %s\n", list_shortdir->element_list[i]->name,
                   list_shortdir->element_list[i]->path);
            return;
        }
    }
    list_shortdir->element_list[list_shortdir->element_count++] = element_shrt;
    printf("%s is set as an alias for %s\n", element_shrt->name, element_shrt->path);
}

// Delete shortdir - del
void delete_shortdir(char *name)
{
    int i;
    int count = list_shortdir->element_count;

    for (i = 0; i < count; i++)
    {
        if (strcmp(list_shortdir->element_list[i]->name, name) == 0)
        {
            list_shortdir->element_list[i] = list_shortdir->element_list[--list_shortdir->element_count];
            printf("Short direction is removed.\n");
            return;
        }
    }
    printf("Short direction is not found.\n");
}

// List shortdir
void print_shortdir()
{
    int i;
    for (i = 0; i < list_shortdir->element_count; i++)
    {
        printf("name: %s, path: %s\n", list_shortdir->element_list[i]->name,
               list_shortdir->element_list[i]->path);
    }
}

// Find shortdir path
char *find_shortdir_path(char *name)
{
    int i;
    for (i = 0; i < list_shortdir->element_count; i++)
    {
        if (strcmp(list_shortdir->element_list[i]->name, name) == 0)
        {
            return list_shortdir->element_list[i]->path;
        }
    }
    return NULL;
}
// Update shortdir file
void update_shortdir_file()
{
    FILE *fp;
    fp = fopen("/home/turkusavran/Desktop/Comp304/operating-systems/Comp304/operating-systems/seashell-project/shortdir.txt", "w+");
    if (fp == NULL)
    {
        printf("Could not open file\n");
        return;
    }
    int i;
    for (i = 0; i < list_shortdir->element_count; i++)
    {
        fprintf(fp, "%s %s\n", list_shortdir->element_list[i]->name,
                list_shortdir->element_list[i]->path);
    }
    fclose(fp);
}

// Change the given word into given color
void changeColor(char *color, char *word)
{
    if (strcmp(color, "b") == 0)
    {
        printf("\x1B[44m"
               "%s"
               "\x1B[0m ",
               word);
    }
    else if (strcmp(color, "r") == 0)
    {
        printf("\x1B[41m"
               "%s"
               "\x1B[0m ",
               word);
    }
    else if (strcmp(color, "g") == 0)
    {
        printf("\x1B[42m"
               "%s"
               "\x1B[0m ",
               word);
    }
}

// Read given file and return as a string
char *readFile(char *document)
{
    FILE *fp;
    fp = fopen(document, "r");

    // char *buff = malloc(222);
    char *buff = (char *)malloc(12 * sizeof(char));
    //fscanf(fp, "%s", buff);

    fread(buff, 222, 1, fp);
    fclose(fp);

    return buff;
}
