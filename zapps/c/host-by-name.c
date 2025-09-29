// @LINK: libmlw
#include "mlw.h"


size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	d_len;
	size_t	s_len;
	size_t	i;

	d_len = 0;
	while (d_len < size && dst[d_len] != '\0')
		d_len++;
	s_len = 0;
	while (src[s_len] != '\0')
		s_len++;
	if (d_len == size)
		return (s_len + size);
	i = 0;
	while (src[i] != '\0' && i + d_len < size - 1)
	{
		dst[d_len + i] = src[i];
		i++;
	}
	dst[d_len + i] = '\0';
	return (d_len + s_len);
}

static int	count_splited(const char *s, char c)
{
	int	count;
	int	i;

	i = 0;
	count = 0;
	while (s[i] != '\0')
	{
		if ((i == 0 && s[0] != c) || (i > 0 && s[i] != c && s[i - 1] == c))
			count++;
		i++;
	}
	return (count);
}

static void	*split_free_all(char **strs)
{
	int	i;

	i = 0;
	while (strs[i] != NULL)
		free(strs[i++]);
	free(strs);
	return (NULL);
}

static char	*dup_the_word(const char *s, char c, int idx)
{
	int		end;
	char	*res;

	end = idx;
	while (s[end] != '\0' && s[end] != c)
		end++;
	res = malloc(sizeof(char) * (end - idx + 1));
	if (res == NULL)
		return (NULL);
	res[0] = '\0';
	ft_strlcat(res, &(s[idx]), end - idx + 1);
	return (res);
}

char	**ft_split(const char *s, char c)
{
	char	**res;
	int		len;
	int		i;
	int		k;

	len = count_splited(s, c);
	res = calloc(len + 1, sizeof(char *));
	if (res == NULL)
		return (NULL);
	i = 0;
	k = 0;
	while (k < len)
	{
		if ((i == 0 && s[0] != c) || (i > 0 && s[i] != c && s[i - 1] == c))
		{
			res[k] = dup_the_word(s, c, i);
			if (res[k] == NULL)
				return ((char **)split_free_all(res));
			k++;
		}
		i++;
	}
	return (res);
}

typedef struct dns_header_t {
	uint16_t xid;
	uint16_t flags;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
} __attribute__((packed)) dns_header_t;


void search(char *domain) {
	char **split = ft_split(domain, '.');
	int total_len = 0;
	for (int i = 0; split[i]; i++)
		total_len += 1 + strlen(split[i]);
	total_len += sizeof(dns_header_t) + 1 + 4;

	mlw_init(MLW_INIT_RAND);
	mlw_udp_t *inst = mlw_udp_open(mlw_ip_from_str("8.8.8.8"), 53);
	uint8_t *packet = malloc(total_len);

	dns_header_t *hdr = (void *)packet;
	hdr->xid = htons(syscall_eth_get_transaction() & 0xFFFF);
	hdr->flags = htons(0x0100);
	hdr->qdcount = htons(1);
	hdr->ancount = 0;
	hdr->nscount = 0;
	hdr->arcount = 0;
	int offset = sizeof(dns_header_t);
	for (int i = 0; split[i]; i++) {
		packet[offset] = strlen(split[i]);
		memcpy(packet + offset + 1, split[i], packet[offset]);
		offset += packet[offset] + 1;
	}
	packet[offset++] = 0;
	*(uint16_t *)&packet[offset] = htons(1);
	offset += 2;
	*(uint16_t *)&packet[offset] = htons(1);
	offset += 2;
	mlw_udp_send(inst, packet, total_len);
}

int main(int, char **argv) {
	search(argv[1]);
	return 0;
}