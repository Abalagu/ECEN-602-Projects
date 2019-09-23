
typedef enum sbcpheadertypes_t
{
    JOIN = 2,
    FWD = 3,
    SEND = 4
} headerType;

typedef enum sbcpattributetype_t
{
    REASON = 1,
    USERNAME = 2,
    CLIENTCOUNT = 3,
    MESSAGE = 4
} attributeType;

typedef struct scbpattribute_t
{
    attributeType type;
    int len;
    int payload;
} sbcpAttribute;

typedef struct sbcpmsg_t
{
    int vrsn;              // protocol version
    headerType type;       // SBCP message type
    int len;               // length of SBCP message
    sbcpAttribute payload; // payload
} sbcpMsg;
