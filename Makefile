CC = cc

CFLAGS = -Wall -Werror -Wextra -MMD -g3

INCLUDES = -I includes/ -I libft/

SRCS_DIR = srcs/

SRCS_FILES = malloc.c test_utils.c utils.c free.c realloc.c

SRCS = $(addprefix $(SRCS_DIR), $(SRCS_FILES))

OBJS = $(SRCS:.c=.o)

DEPS = $(OBJS:.o=.d)

LIBFT_PATH = libft/
LIBFT_HEADERS_PATH = libft/includes/
LIBFT = -L $(LIBFT_PATH) -lft

ifeq ($(HOSTTYPE),)
	HOSTTYPE = $(shell uname -m)_$(shell uname -s)
endif

#NAME = libft_malloc$(HOSTTYPE).so
NAME = malloc_test

all: $(NAME)

$(NAME): $(OBJS) $(H_DEPS)
	make -C $(LIBFT_PATH) all
#	$(CC) $(CFLAGS) -shared $(OBJS) $(LIBFT) -o $(NAME)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -fPIC $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean: 
	make -C $(LIBFT_PATH) clean
	rm -rf $(OBJS) $(DEPS)

fclean: clean 
	rm -rf $(LIBFT_PATH)libft.a
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
