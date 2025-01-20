CC = cc

CFLAGS = -Wall -Werror -Wextra -MMD

INCLUDES = -I includes/

SRCS_DIR = srcs/

SRCS_FILES = malloc.c

SRCS = $(addprefix $(SRCS_DIR), $(SRCS_FILES))

OBJS = $(SRCS:.c=.o)

DEPS = $(OBJS:.o=.d)

NAME = libft_malloc$(HOSTTYPE).so

all: 

$(NAME): $(OBJS) $(H_DEPS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

-include $(DEPS)

clean: rm -rf $(OBJS) $(DEPS)

fclean: clean 
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
