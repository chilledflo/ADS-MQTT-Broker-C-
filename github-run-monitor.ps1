param(
  [string]$Repo = 'chilledflo/ADS-MQTT-Broker-C-',
  [long]$RunId
)

if (-not $RunId) {
  Write-Host "RunId missing. Exiting." -ForegroundColor Red
  exit 1
}

$runUrl = "https://api.github.com/repos/$Repo/actions/runs/$RunId"
$artifactUrl = "https://api.github.com/repos/$Repo/actions/runs/$RunId/artifacts"
$outFile = Join-Path $PSScriptRoot "GITHUB-BUILD-RESULT.txt"

Write-Host "Monitoring GitHub Actions run $RunId for $Repo..." -ForegroundColor Cyan

$lastStatus = ''
$start = Get-Date

while ($true) {
  try {
    $run = Invoke-RestMethod -Uri $runUrl -UseBasicParsing -Headers @{ 'User-Agent'='vscode-copilot' }
    if ($run.status -ne $lastStatus) {
      $lastStatus = $run.status
      Write-Host ("[{0}] Status: {1}" -f (Get-Date -Format 'HH:mm:ss'), $run.status) -ForegroundColor Yellow
    }

    if ($run.status -eq 'completed') {
      $duration = New-TimeSpan -Start $start -End (Get-Date)
      Write-Host ("Completed with conclusion: {0} (duration: {1}m {2}s)" -f $run.conclusion, [int]$duration.TotalMinutes, $duration.Seconds) -ForegroundColor Green
      $artifacts = $null
      try { $artifacts = Invoke-RestMethod -Uri $artifactUrl -UseBasicParsing -Headers @{ 'User-Agent'='vscode-copilot' } } catch {}

      $lines = @()
      $lines += "Run: $($run.name)"
      $lines += "Run URL: $($run.html_url)"
      $lines += "Status: $($run.status)"
      $lines += "Conclusion: $($run.conclusion)"
      $lines += "Duration: $([int]$duration.TotalMinutes)m $($duration.Seconds)s"
      if ($artifacts.count -gt 0 -and $artifacts.artifacts.Count -gt 0) {
        $lines += "Artifacts:"
        foreach ($a in $artifacts.artifacts) { $lines += " - $($a.name) (expired: $($a.expired))" }
      } else {
        $lines += "Artifacts: (visit run URL to download)"
      }
      $lines | Out-File -FilePath $outFile -Encoding utf8
      break
    }
  } catch {
    Write-Host "API error: $($_.Exception.Message)" -ForegroundColor Red
  }
  Start-Sleep -Seconds 20
}

Write-Host "Result saved to: $outFile" -ForegroundColor Cyan
