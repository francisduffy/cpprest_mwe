while true; do
ts=$(date +"%m/%d/%Y %H:%M:%S.%N")
mem_usage=$(docker stats --no-stream --format "{{.MemUsage}}" mwe_test)
echo "$ts,$mem_usage" >> docker_mem.log
done