#include "directory.h"
#include <sys/socket.h>
#include <string.h>
#include "csapp.h"
#include "debug.h"

/*
 * The directory maintains a mapping from handles (i.e. user names)
 * to client info, which includes the client mailbox and the socket
 * file descriptor over which commands from the client are received.
 * The directory is used when sending a message, in order to find
 * the destination mailbox that corresponds to a particular recipient
 * specified by their handle.
 */
typedef struct directoryNode directoryNode;
pthread_mutex_t mutex;
struct directoryNode {
    MAILBOX *mail;
    int fd;
    char* name;
    directoryNode *next;
    directoryNode *previous;
};

typedef struct directory {

    directoryNode *head;
    int defunct; //if 1 then defunct
} DIRECTORY;
DIRECTORY *dir = NULL;

/*
 * Initialize the directory.
 */
void dir_init(void)
{
    pthread_mutex_init(&mutex, NULL);
    dir = malloc(sizeof(DIRECTORY));
    dir->defunct = 0;
    dir->head = NULL;
}

/*
 * Shut down the directory.
 * This marks the directory as "defunct" and shuts down all the client sockets,
 * which triggers the eventual termination of all the server threads.
 */
void dir_shutdown(void)
{
    pthread_mutex_lock(&mutex);
    dir->defunct = 1;
    directoryNode *current = dir->head;
    //directoryNode *previous;
    while (current != NULL)
    {
        shutdown(current->fd, SHUT_RDWR);
        //mb_shutdown(current->mail);
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);
}

/*
 * Finalize the directory.
 *
 * Precondition: the directory must previously have been shut down
 * by a call to dir_shutdown().
 */
void dir_fini(void)
{
    pthread_mutex_lock(&mutex);
    directoryNode *current = dir->head;
    directoryNode *nodeToFree = dir->head;
    while (current != NULL)
    {
        current = nodeToFree->next;
        free(nodeToFree->name);
        //free(nodeToFree->mail);
        mb_unref(nodeToFree->mail);
        mb_shutdown(nodeToFree->mail);
        free(nodeToFree);
        nodeToFree = current;
    }
    pthread_mutex_unlock(&mutex);

}

/*
 * Register a handle in the directory.
 *   handle - the handle to register
 *   sockfd - file descriptor of client socket
 *
 * Returns a new mailbox, if handle was not previously registered.
 * Returns NULL if handle was already registered or if the directory is defunct.
 */
MAILBOX *dir_register(char *handle, int sockfd)
{
    pthread_mutex_lock(&mutex);
    int returnNull = 0;
    if (dir->defunct == 1)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    directoryNode *current = dir->head;
    directoryNode *previous = NULL;
    while(current != NULL)
    {
        if (strcmp(handle, current->name) == 0)
        {
            returnNull = 1;
        }
        previous = current;
        current = current->next;

    }
    if (returnNull == 1)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    directoryNode* newGuy = malloc(sizeof(directoryNode));
    newGuy->name = strdup(handle);
    newGuy->fd = sockfd;
    newGuy->mail = mb_init(newGuy->name);
    mb_ref(newGuy->mail);
    if (dir->head == NULL)
    {
        newGuy->next = NULL;
        newGuy->previous = NULL;
        dir->head = newGuy;
    }
    else
    {
        newGuy->previous = previous;
        newGuy->next = NULL;
        previous->next = newGuy;
    }


    pthread_mutex_unlock(&mutex);
    return newGuy->mail;



}

/*
 * Unregister a handle in the directory.
 * The associated mailbox is removed from the directory and shut down.
 */
void dir_unregister(char *handle)
{
    pthread_mutex_lock(&mutex);
    directoryNode *current = dir->head;
    directoryNode *previous = NULL;
    int found = 0;
    while (current != NULL)
    {
        if (strcmp(current->name, handle) == 0)
        {
            found = 1;
            break;
        }
        previous = current;
        current = current->next;
    }
    if ( found == 0)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }
    if (previous == NULL) //meaning the user was found at the beginning of the list
    {
        dir->head = current->next;
        if (current->next != NULL)
        {
             current->next->previous = NULL;
        }

    }
    else
    {
         previous->next = current->next;
         if (current->next != NULL)
         {
            current->next->previous = previous;
         }

    }
    mb_unref(current->mail);
    mb_shutdown(current->mail);
    free(current->name);
    free(current);
    pthread_mutex_unlock(&mutex);

}

/*
 * Query the directory for a specified handle.
 * If the handle is not registered, NULL is returned.
 * If the handle is registered, the corresponding mailbox is returned.
 * The reference count of the mailbox is increased to account for the
 * pointer that is being returned.  It is the caller's responsibility
 * to decrease the reference count when the pointer is ultimately discarded.
 */
MAILBOX *dir_lookup(char *handle)
{
    pthread_mutex_lock(&mutex);
    directoryNode *current = dir->head;
    int found = 0;
    while (current != NULL)
    {
        if (strcmp(current->name, handle) == 0)
        {
            found = 1;
            break;
        }
        current = current->next;
    }
    if ( found == 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    else
    {
        mb_ref(current->mail);
        pthread_mutex_unlock(&mutex);
        return current->mail;
    }
}

/*
 * Obtain a list of all handles currently registered in the directory.
 * Returns a NULL-terminated array of strings.
 * It is the caller's responsibility to free the array and all the strings
 * that it contains.
 */
char **dir_all_handles(void)
{
    pthread_mutex_lock(&mutex);
    int counter = 0;
    directoryNode *current = dir->head;
    while ( current != NULL)
    {
        counter++;
        current = current->next;
    }
    char **handles = malloc(counter * sizeof(char *) + 1);
    counter = 0;
    current = dir->head;
    while ( current != NULL)
    {
        handles[counter] = strdup(current->name);
        debug("current->name = %s", current->name);
        current = current->next;
        counter++;
    }
    handles[counter] = NULL;

    pthread_mutex_unlock(&mutex);
    return handles;
}


