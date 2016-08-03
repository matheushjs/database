cd test_environment
valgrind --leak-check=full --show-leak-kinds=all ./shell < input
