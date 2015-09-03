#include <errno.h> /* errno */
#include <stdio.h> /* fprintf, stderr, stdcout */
#include <stdlib.h> /* realloc */
#include <string.h> /* strerror */
#include <unistd.h> /* close */
#include <netinet/ip.h> /* socket functions */
#include "ip_fw.h" /* ipfw types */

#define IP_FW_RULE_NUM_SENTINEL 65535

/**
 * @brief Get the ip firewall rules.  Caller receives ownership
 *        of the returned buffer.
 *
 * @param sockfd raw socket fd on which sockopt calls will be made.
 * @return buffer containing ip_fw rules on success (caller receives
 *         ownership); otherwise NULL.
 */
struct ip_fw* get_rules(int sockfd)
{
  struct ip_fw* rules = NULL;
  void* buffer = NULL;
  int buffer_size = 0;
  int value_size = 0;

  /* When using getsockopt() the optlen provides the size of buffer,
   * but upon return it contains amount of bytes used in the buffer.
   * Keep growing the buffer and requesting data until the buffer is
   * large enough to store the entire socket option value. */
  while (value_size == buffer_size)
  {
    /* Grow buffer.  On failure, return NULL; */
    buffer_size = buffer_size * 2 + 1 + sizeof(*rules);
    rules = buffer; /* Keep handle before realloc */
    buffer = realloc(buffer, buffer_size);
    if (!buffer) {
      free(rules);
      return NULL;
    }

    /* Request for rules. */
    rules = buffer;
    rules->version = IP_FW_CURRENT_API_VERSION;
    value_size = buffer_size;
    if (0 > getsockopt(sockfd, IPPROTO_IP, IP_FW_GET, buffer,
          (socklen_t*) &value_size))
    {
      fprintf(stderr, "IP_FW_GET failed: %s\n", strerror(errno));
      free(rules);
      return NULL;
    }
  }

  return rules;
}

/**
 * @brief Delete all ipfw deny rules.
 *
 * @param sockfd raw socket fd on which sockopt calls will be made.
 * @return 0 on success; otherwise -1.
 */
int delete_all_deny_rules(int sockfd)
{
  /* Get all rules. */
  struct ip_fw* rules = get_rules(sockfd);
  if (!rules)
  {
    return -1;
  }

  /* Delete all deny rules. */
  unsigned int deleted_count = 0;
  for (int i = 0; rules[i].fw_number < IP_FW_RULE_NUM_SENTINEL; ++i)
  {
    /* If this is not a deny rule, then continue to next. */
    if ((IP_FW_F_COMMAND & rules[i].fw_flg) != IP_FW_F_DENY)
    {
      continue;
    }

    /* This is a deny rule, so delete it. */
    if (0 > setsockopt(sockfd, IPPROTO_IP, IP_FW_DEL, &rules[i], sizeof(*rules)))
    {
      fprintf(stderr, "Failed to delete rule %i: %s\n",
        rules[i].fw_number, strerror(errno));
      free(rules);
      return -1;
    }
    ++deleted_count;
  }

  if (deleted_count == 0)
  {
    fprintf(stdout, "No rules deleted.  Connect to VPN first.\n");
  }

  free(rules);
  return 0;
}

int main()
{
  /* ipfw can only be manipulated via a raw socket, and raw sockets
   * require elevated permissions. */
  int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if (sockfd < 0)
  {
    fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
    return -1;
  }

  /* Delete all ipfw deny rules. */
  int result = delete_all_deny_rules(sockfd);
  close(sockfd);
  return result;
}
