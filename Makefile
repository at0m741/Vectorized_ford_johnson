NAME_BASE = PmergeMe
NAME = $(NAME_BASE)-$(MODE)

CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -flto
MODE ?= fast

UNAME_M := $(shell uname -m)

ifeq ($(UNAME_M),x86_64)
	CXXFLAGS += -march=native -mavx2 -mprefer-vector-width=256
endif

ifeq ($(MODE),fast)
	CXXFLAGS += -DPMERGEME_MODE_FAST
	CXXFLAGS += -DPMERGEME_SIMD_UNROLL=0
else ifeq ($(MODE),fast_unroll)
	CXXFLAGS += -DPMERGEME_MODE_FAST
	CXXFLAGS += -DPMERGEME_SIMD_UNROLL=1
else ifeq ($(MODE),mincmp)
	CXXFLAGS += -DPMERGEME_MODE_MINCMP
	CXXFLAGS += -DPMERGEME_SIMD_UNROLL=0
else
$(error Unsupported MODE=$(MODE). Use MODE=fast, fast_unroll or mincmp)
endif

SRC = main.cpp \
	  Jacobsthal.cpp \
	  PmergeMe.cpp \
	  utils.cpp \
	  compare.cpp \
	  PmergeMe2.cpp

OBJ_DIR = obj/$(MODE)
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

ifeq ($(DEBUG),true)
	CXXFLAGS += -g -DDEBUG
endif

ifeq ($(AVX512),1)
ifeq ($(UNAME_M),x86_64)
	CXXFLAGS += -mavx512f -mavx512dq -mavx512cd -mavx512bw -mavx512vl
endif
endif

all: $(NAME_BASE)

opti: CXXFLAGS += -O3
opti: re

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(NAME_BASE): $(NAME)
	cp $(NAME) $(NAME_BASE)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME_BASE) \
		$(NAME_BASE)-fast \
		$(NAME_BASE)-fast_unroll \
		$(NAME_BASE)-mincmp

re: fclean all

.PHONY: all clean fclean re opti
