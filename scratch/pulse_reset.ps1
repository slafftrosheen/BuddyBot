$port = New-Object System.IO.Ports.SerialPort "COM20", 115200, None, 8, One
$port.ReadTimeout = 3000
try {
    $port.Open()
    # Pulse EN (RTS) while IO0 (DTR) is HIGH (DtrEnable = $false)
    $port.DtrEnable = $false
    $port.RtsEnable = $true
    Start-Sleep -Milliseconds 100
    $port.RtsEnable = $false
    Start-Sleep -Milliseconds 500
    
    if ($port.BytesToRead -gt 0) {
        $data = $port.ReadExisting()
        Write-Output "--- NORMAL BOOT LOG ---"
        Write-Output $data
    } else {
        Write-Output "Waiting for boot output..."
        Start-Sleep -Seconds 2
        if ($port.BytesToRead -gt 0) {
            $data = $port.ReadExisting()
            Write-Output $data
        }
    }
} catch {
    Write-Output "Serial Error: $_"
} finally {
    if ($port.IsOpen) {
        $port.Close()
    }
}
