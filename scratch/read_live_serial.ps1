$port = New-Object System.IO.Ports.SerialPort "COM20", 115200, None, 8, One
$port.ReadTimeout = 4000
try {
    $port.Open()
    Start-Sleep -Seconds 3
    if ($port.BytesToRead -gt 0) {
        $data = $port.ReadExisting()
        Write-Output "--- LIVE SERIAL OUTPUT ---"
        Write-Output $data
    } else {
        Write-Output "No data received on COM20 (buffer empty)"
    }
} catch {
    Write-Output "Serial Error: $_"
} finally {
    if ($port.IsOpen) {
        $port.Close()
    }
}
