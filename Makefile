NAME = PmergeMe

CC = c++

CFLAGS = -Wall -Wextra -Werror -march=native -mavx2 -mprefer-vector-width=256 -flto -funroll-loops

SRC = main.cpp \
	  Jacobsthal.cpp \
	  PmergeMe.cpp \
	  utils.cpp	\
	  compare.cpp \
	  compare2.cpp \
	  PmergeMe2.cpp \
	  Jacobsthal2.cpp \

OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

ifeq ($(DEBUG), true)
	CFLAGS += -g -D DEBUG
endif

ifeq ($(AVX512), 1)
	CFLAGS += -mavx512f -mavx512dq -mavx512cd -mavx512bw -mavx512vl
endif

all: $(NAME)

opti: CFLAGS += -O3 -mtune=native
opti: re

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
