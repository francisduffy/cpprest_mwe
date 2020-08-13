function Get-TimeStamp {
    return "{0:MM/dd/yyyy} {0:HH:mm:ss.fff}" -f (Get-Date)
}

while ($true) {
    $mem_usage = docker stats --no-stream --format "{{.MemUsage}}" mwe_test
    Write-Output "$(Get-TimeStamp),$mem_usage" | Out-file docker_mem.log -append
    # start-sleep -seconds 1
}
