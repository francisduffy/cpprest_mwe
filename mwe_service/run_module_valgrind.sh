# Add valgrind options
if [[ -z "${VALGRIND_OPTIONS}" ]]; then
  VALGRIND_OPTIONS="--tool=memcheck --verbose --leak-check=full --show-reachable=yes --undef-value-errors=yes --show-leak-kinds=all --track-origins=yes --child-silent-after-fork=yes --trace-children=yes --gen-suppressions=all --xml=yes --xml-file=/app/log/ps_memcheck.xml --log-file=/app/log/ps_memcheck.txt"
fi

# Run command
valgrind ${VALGRIND_OPTIONS} /app/bin/mwe_service 0.0.0.0
