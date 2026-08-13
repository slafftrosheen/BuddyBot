$port = New-Object System.IO.Ports.SerialPort "COM20", 115200, None, 8, One
$port.ReadTimeout = 3000
try {
    $port.Open()
    $port.DtrEnable = $true
    $port.RtsEnable = $true
    Start-Sleep -Milliseconds 200
    $port.DtrEnable = $false
    $port.RtsEnable = $false
    Start-Sleep -Seconds 2
    if ($port.BytesToRead -gt 0) {
        $data = $port.ReadExisting()
        Write-Output "--- SERIAL OUTPUT START ---"
        Write-Output $data
        Write-Output "--- SERIAL OUTPUT END ---"
    } else {
        Write-Output "No data received on COM20"
    }
} catch {
    Write-Output "Serial Error: $_"
} finally {
    if ($port.IsOpen) {
        $port.Close()
    }
}
