#include "protocol.h"
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "debug.h"

/*
 * Send a packet with a specified header and payload.
 *   fd - file descriptor on which packet is to be sent
 *   hdr - the packet header, with multi-byte fields in host byte order
 *   payload - pointer to packet payload, or NULL, if none.
 *
 * Multi-byte fields in the header are converted to network byte order
 * before sending.  The header structure passed to this function may
 * be modified as a result.
 *
 * On success, 0 is returned.
 * On error, -1 is returned and errno is set.
 */
int proto_send_packet(int fd, bvd_packet_header *hdr, void *payload)
{
    //uint8_t type;          // Type of the packet
    //uint32_t payload_length;       // Length of payload
    //uint32_t msgid;                // Unique ID of message to which packet pertains
    //uint32_t timestamp_sec;        // Seconds field of time packet was sent
    //uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent
    //bvd_packet_header newHeader;
   // new.type = hdr->type;
    debug("Header type = %d", hdr->type);
    uint32_t hostPayloadLength = hdr->payload_length;
    hdr->payload_length = htonl(hdr->payload_length);
    hdr->msgid = htonl(hdr->msgid);
    struct timespec p;
    clockid_t cid = CLOCK_MONOTONIC;
    clock_gettime(cid, &p);
    hdr->timestamp_sec = htonl(p.tv_sec);
    hdr->timestamp_nsec = htonl(p.tv_nsec);
    //hdr->type = htonl(hdr->type)

   // size_t size = sizeof(hdr->type) + sizeof(hdr->payload_length) + sizeof(hdr->msgid)
    //+ sizeof(hdr->timestamp_sec) + sizeof(hdr->timestamp_nsec);

    //void * buffer = malloc(sizeof(bvd_packet_header));
    //void * beginOfBuffer = buffer;
    /*
    memcpy(buffer, &newHeader.type, sizeof(newHeader.type));
    buffer = buffer + sizeof(newHeader.type);
    memcpy(buffer, &newHeader.payload_length, sizeof(newHeader.payload_length));
    buffer = buffer + sizeof(newHeader.payload_length);
    memcpy(buffer, &newHeader.msgid, sizeof(newHeader.msgid));
    buffer = buffer + sizeof(newHeader.msgid);
    memcpy(buffer, &newHeader.timestamp_sec, sizeof(newHeader.timestamp_sec));
    buffer = buffer + sizeof(newHeader.timestamp_sec);
    memcpy(buffer, &newHeader.timestamp_nsec, sizeof(newHeader.timestamp_nsec));
    */

    //memcpy(buffer, &newHeader, sizeof(newHeader));
    if (rio_writen(fd, hdr, sizeof(bvd_packet_header)) != sizeof(bvd_packet_header))
    {
        debug("Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
   /* if (rio_writen(fd, beginOfBuffer, sizeof(newHeader.type)) != sizeof(newHeader.type))
    {
        fprintf(stderr, "Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
    beginOfBuffer += sizeof(newHeader.type);
        if (rio_writen(fd, beginOfBuffer, sizeof(newHeader.payload_length)) != sizeof(newHeader.payload_length))
    {
        fprintf(stderr, "Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
    beginOfBuffer += sizeof(newHeader.payload_length);
    if (rio_writen(fd, beginOfBuffer, sizeof(newHeader.msgid)) != sizeof(newHeader.msgid))
    {
        fprintf(stderr, "Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
    beginOfBuffer += sizeof(newHeader.msgid);

    if (rio_writen(fd, beginOfBuffer, sizeof(newHeader.timestamp_sec)) != sizeof(newHeader.timestamp_sec))
    {
        fprintf(stderr, "Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
    beginOfBuffer += sizeof(newHeader.timestamp_sec);

    if (rio_writen(fd, beginOfBuffer, sizeof(newHeader.timestamp_nsec)) != sizeof(newHeader.timestamp_nsec))
    {
        fprintf(stderr, "Failed rio_writen: %s\n", strerror(errno));
        return -1;
    }
    */




    if (payload == NULL)
    {
        return 0;
    }
    else
    {
        if (rio_writen(fd, payload, hostPayloadLength) != hostPayloadLength)
        {
            debug("Failed rio_writen: %s\n", strerror(errno));
            return -1;

        }
        return 0;
    }
    //free(beginOfBuffer);




}

/*
 * Receive a packet, blocking until one is available.
 *  fd - file descriptor from which packet is to be received
 *  hdr - pointer to caller-supplied storage for packet header
 *  payload - variable into which to store payload pointer
 *
 * The returned header has its multi-byte fields in host byte order.
 *
 * If the returned payload pointer is non-NULL, then the caller
 * is responsible for freeing the storage.
 *
 * On success, 0 is returned.
 * On error, -1 is returned, payload and length are left unchanged,
 * and errno is set.
 */
int proto_recv_packet(int fd, bvd_packet_header *hdr, void **payload)
{
    //uint8_t type;          // Type of the packet
    //uint32_t payload_length;       // Length of payload
    //uint32_t msgid;                // Unique ID of message to which packet pertains
    //uint32_t timestamp_sec;        // Seconds field of time packet was sent
    //uint32_t timestamp_nsec;       // Nanoseconds field of time packet
    //printf("Succesfully read the header in proto_recv_packet");
    debug("we are now right before the line that reads the header in proto_recv_packet");
    if (rio_readn(fd, hdr, sizeof(bvd_packet_header)) < 0)
    {
        debug("Failed rio_readn: %s\n", strerror(errno));
        return -1;
    }
    debug("Successfully read the header in proto_recv_packet");

    //hdr->type = ntohl(hdr->type);
    hdr->payload_length = ntohl(hdr->payload_length);
    hdr->msgid = ntohl(hdr->msgid);
    hdr->timestamp_sec = ntohl(hdr->timestamp_sec);
    hdr->timestamp_nsec = ntohl(hdr->timestamp_nsec);
    if (hdr->payload_length > 0)
    {
        *payload = malloc(hdr->payload_length);
        debug("payload_length = %d\n", hdr->payload_length);
        debug("right before the payload read address = %p", *payload);
        int result = rio_readn(fd, *payload, hdr->payload_length);
        if (result < 0)
        {
            debug("Failed rio_readn: %s\n", strerror(errno));
            return -1;
        }
        else if (result == 0)
        {
            return -1; //EOF
        }
        debug("Address of buffer i.e. payload = %p", *payload);
        //debug("Contents of payload: %s", (char *)buffer);
        //payload = buffer;

        //debug("payload itself = %s", (char *) *payload);
    }
    else
    {
        payload = NULL;
    }
    return 0;



}

