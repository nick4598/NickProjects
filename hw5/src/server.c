#include "server.h"
#include "protocol.h"
#include "directory.h"
#include <string.h>
#include "csapp.h"
#include <sys/socket.h>

typedef struct FDMBNode FDMBNode;
typedef struct fd_and_mb fd_and_mb;
struct FDMBNode
{
    fd_and_mb *fdmb;
    FDMBNode *next;
    FDMBNode *prev;
};
FDMBNode *head = NULL;

//FDMBNode *tail = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_init(&lock, NULL);
THREAD_COUNTER *thread_counter;

void addNode(FDMBNode *node)
{
    pthread_mutex_lock(&lock);
    FDMBNode *current = head;
    FDMBNode *previous = NULL;
    while (current != NULL)
    {
        previous = current;
        current = current->next;
    }

    if (head == NULL)
    {
        node->next = NULL;
        node->prev = NULL;
        head = node;
    }
    else
    {
        node->prev = previous;
        node->next = NULL;
        previous->next = node;
    }
    pthread_mutex_unlock(&lock);


}
FDMBNode* removeNode(int fd)
{
    pthread_mutex_lock(&lock);
    FDMBNode *current = head;
    FDMBNode *previous = NULL;
    int found = 0;
    while (current != NULL)
    {
        if (current->fdmb->fd == fd)
        {
            found = 1;
            break;
        }
        previous = current;
        current = current->next;

    }
    if ( found == 0)
    {
        return NULL;
    }
    FDMBNode* nodeToReturn = current;
    if (previous == NULL) //meaning the user was found atbeginning of list
    {
        head = current->next;
        if (current->next != NULL)
        {
            current->next->prev = NULL;
        }
    }
    else
    {
        previous->next = current->next;
        if (current->next != NULL)
        {
            current->next->prev = previous;
        }
    }
    pthread_mutex_unlock(&lock);
    return nodeToReturn;

}

void sendNoticePacket(int fd, bvd_packet_header *hdr, bvd_packet_type type, int payload_length, void* payload)
{
        //bvd_packet_header *newHeader = malloc(sizeof(bvd_packet_header));

        hdr->type = type;
        hdr->payload_length = payload_length;
        hdr->msgid = hdr->msgid;
        if (type == BVD_ACK_PKT || type == BVD_NACK_PKT)
        {
            hdr->msgid = 0;
        }
        proto_send_packet(fd, hdr, payload);
        //free(hdr); //might fuck eeverything up
        //free(newHeader); //also might fuck
        if (payload != NULL)
        {
            //free(payload);
        }

}
MAILBOX *FDLookup(int fd)
{
    pthread_mutex_lock(&lock);
    FDMBNode *current = head;
    int found = 0;
    while (current != NULL)
    {
        if (fd == current->fdmb->fd)
        {
            found = 1;
            break;
        }
        current = current->next;
    }
    if (found == 0)
    {
        pthread_mutex_unlock(&lock);
        return NULL;
    }
    else
    {
        mb_ref(current->fdmb->mb);
        pthread_mutex_unlock(&lock);
        return current->fdmb->mb;
    }
    //pthread_mutex_unlock(&lock);
}
/*struct FDMBNode
{
    fd_and_mb *fdmb;
    FDMBNode *next;
    FDMBNode *prev;
};
void sendNoticeToMailbox(int fd, MAILBOX )
{

}
*/

void discard_hook(MAILBOX_ENTRY *entry)
{
    if (entry->type == NOTICE_ENTRY_TYPE)
    {
        return;
    }
    if (entry->content.message.from == NULL)
    {
        return;
    }
    mb_add_notice(entry->content.message.from, BOUNCE_NOTICE_TYPE, entry->content.message.msgid, NULL, 0);

}

void handle_packet(int fd, bvd_packet_header *hdr, void **payload)
{
    pthread_t thread1;
    if (hdr->type == BVD_LOGIN_PKT)
    {
        char *pos;
        if ((pos = strchr(*payload, '\n')) != NULL)
        {
            pos++;
            *pos = '\0';
        }
        else
        {
            sendNoticePacket(fd,hdr, BVD_NACK_PKT, 0, NULL);
            return;
        }
        MAILBOX *mb = dir_register(*payload, fd);
        if (mb == NULL)
        {
            sendNoticePacket(fd,hdr, BVD_NACK_PKT, 0, NULL);
        }
        else
        {
            mb_set_discard_hook(mb, discard_hook);
            fd_and_mb *fdmb = malloc(sizeof(fd_and_mb));
            fdmb->fd = fd;
            fdmb->mb = mb;
            FDMBNode *node = malloc(sizeof(FDMBNode));
            node->fdmb = fdmb;
            addNode(node);
            //sendNoticePacket(fd, hdr, BVD_ACK_PKT, 0, NULL);
            mb_add_notice(mb, ACK_NOTICE_TYPE, hdr->msgid, NULL, 0);

             fd_and_mb *fdmb2 = malloc(sizeof(fd_and_mb));
             fdmb2->fd = fd;
             fdmb2->mb = mb;
             int success = pthread_create(&thread1, NULL, bvd_mailbox_service,
            (void *)fdmb2);
        //close(connfd);
        //sleep(2000);
            if (success != 0)
            {
                fprintf(stderr, "Unable to create thread! Error: %d", success);
            }
        }

    }
    else if(hdr->type == BVD_LOGOUT_PKT)
    {
        FDMBNode *node = removeNode(fd);
        if (node == NULL)
        {
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        else
        {
            dir_unregister(mb_get_handle(node->fdmb->mb));
            free(node->fdmb);
            free(node);
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
    }
    else if(hdr->type == BVD_USERS_PKT)
    {
        //bvd_packet_header *newHdr = malloc(sizeof(bvd_packet_header));
        char** all_handles = dir_all_handles();
        char** pos = all_handles;

        int counter = 0;
        //int i = 0;
        while (pos[0] != NULL)
        {
            //int size = strlen(pos[0]);
            counter = counter + strlen(pos[0]);
            pos++;
        }
        pos = all_handles;
        char * handles = malloc(counter + 1);
        while (pos[0] != NULL)
        {
            strcat(handles, pos[0]);
            pos++;
        }
        handles[counter +1] ='\0';
        MAILBOX *mb = FDLookup(fd);
        if (mb != NULL)
        {
            mb_add_notice(mb, ACK_NOTICE_TYPE, hdr->msgid, (void *)handles, counter);
            mb_unref(mb);
        }
        else
        {
            sendNoticePacket(fd, hdr, BVD_ACK_PKT, counter, (void *)handles);
        }


    }
    else if(hdr->type == BVD_SEND_PKT)
    {
        char *pos;
        pos = strchr(*payload, '\n');
        int index = pos - (char *)*payload;
        char *handle = malloc(index + 2);
        strncpy(handle, *payload, index + 1);
        handle[index + 1] = '\0';
        ((char *)(*payload))[hdr->payload_length] = '\0';

        char *senderHandle;
        MAILBOX *receiverMB = dir_lookup(handle);
        MAILBOX *senderMB = FDLookup(fd);
        if (senderMB != NULL)
        {
            char *sender = mb_get_handle(senderMB);
            senderHandle = malloc(strlen(sender) + strlen(pos) + 1);
            memcpy(senderHandle, sender, strlen(sender));
            strcat(senderHandle, pos + 1);
        }
        if (receiverMB == NULL)
        {
            if (senderMB != NULL)
            {
                mb_add_notice(senderMB, NACK_NOTICE_TYPE, hdr->msgid, NULL, 0);
                //sendNoticePacket(fd, hdr, BVD_NACK_PKT, 0, NULL);
                mb_unref(senderMB);
            }
            else
            {
                sendNoticePacket(fd, hdr, BVD_NACK_PKT, 0, NULL);
            }

        }
        else
        {

            if (senderMB == NULL)
            {
                sendNoticePacket(fd, hdr, BVD_NACK_PKT, 0, NULL);
            }
            else
            {
                //strcat(senderHandle, pos);

                mb_add_message(receiverMB, hdr->msgid, senderMB, senderHandle, hdr->payload_length);
                sendNoticePacket(fd, hdr, BVD_ACK_PKT, 0, NULL);
                mb_unref(senderMB);
            }
            mb_unref(receiverMB);
        }
    }
}

/*
 * Thread function for the thread that handles client requests.
 *
 * The arg pointer point to the file descriptor of client connection.
 * This pointer must be freed after the file descriptor has been
 * retrieved.
 */
void *bvd_client_service(void *arg)
{
    tcnt_incr(thread_counter);

    int fd = *((int *)arg);
    free(arg);

    bvd_packet_header *hdr = malloc(sizeof(bvd_packet_header));
    void **payload = malloc(sizeof(void *));
    while (1)
    {
        //bvd_packet_header *hdr = malloc(sizeof(bvd_packet_header));
        //void **payload = malloc(sizeof(void **));
        int success = proto_recv_packet(fd, hdr, payload);
        if ( success == -1)
        {
            break;
        }
        handle_packet(fd, hdr, payload);
       // free(hdr);
        //free(payload);
    }
    free(hdr);
    free(payload);
    tcnt_decr(thread_counter);
    return NULL;

}


/*
 * Thread function for the thread that handles client requests.
 *
 * The arg pointer points to the file descriptor of client connection.
 * This pointer must be freed after the file descriptor has been
 * retrieved.
 */

/*
 * Once the file descriptor and mailbox have been retrieved,
 * this structure must be freed.
 */
void *bvd_mailbox_service(void *arg)
{
    tcnt_incr(thread_counter);
    fd_and_mb *fdmb = (fd_and_mb*)(arg);
    int fd = fdmb->fd;
    MAILBOX *mb = fdmb->mb;
    free(fdmb);

     bvd_packet_header *hdr = malloc(sizeof(bvd_packet_header));
    while(1)
    {
        MAILBOX_ENTRY *entry = mb_next_entry(mb);
        if (entry == NULL)
        {
            mb_unref(mb);
            tcnt_decr(thread_counter);
            pthread_exit(NULL);
        }
        else
        {
            if (entry->type == MESSAGE_ENTRY_TYPE)
            {
                //send rrcpt packet to sender
                //DLVR packet to receiver (us) and client will display info prob
                mb_add_notice(entry->content.message.from, RRCPT_NOTICE_TYPE,
                    entry->content.message.msgid, NULL, 0);
                hdr->type = BVD_DLVR_PKT;
                hdr->payload_length = entry->length;
                hdr->msgid = entry->content.message.msgid;
                proto_send_packet(fd, hdr, entry->body);
                //free(hdr);
                //free(&(entry->content.message));
                //free(entry);

            }
            else if (entry->type == NOTICE_ENTRY_TYPE)
            {
                //bvd_packet_header *hdr = malloc(sizeof(bvd_packet_header));
                NOTICE_TYPE type = entry->content.notice.type;
                if (type == ACK_NOTICE_TYPE)
                {
                    hdr->type = BVD_ACK_PKT;
                }
                else if(type == NACK_NOTICE_TYPE)
                {
                    hdr->type = BVD_NACK_PKT;
                }
                else if(type == BOUNCE_NOTICE_TYPE)
                {
                    hdr->type = BVD_BOUNCE_PKT;
                }
                else if(type == RRCPT_NOTICE_TYPE)
                {
                    hdr->type = BVD_RRCPT_PKT;
                }
                //hdr->type = entry->content.notice.NOTICE_TYPE;
                hdr->payload_length = entry->length;
                hdr->msgid = entry->content.notice.msgid;
                proto_send_packet(fd, hdr, entry->body);
                //free(hdr);
                //free(&(entry->content.notice));
                //free(entry);
            }
        }
    }
    free(hdr);
}


