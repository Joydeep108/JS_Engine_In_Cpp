$jsEngine = Join-Path $PSScriptRoot "..\js_engine.exe"
$testDir = $PSScriptRoot
$passed = 0
$failed = 0
$errors = @()

Get-ChildItem -Path $testDir -Filter "*.js" | Sort-Object Name | ForEach-Object {
    $jsFile = $_.FullName
    $expectedFile = $jsFile -replace '\.js$', '.expected'
    $testName = $_.BaseName
    
    if(!(Test-Path $expectedFile)) {
        Write-Host "SKIP $testName (no .expected file)" -ForegroundColor Yellow
        return
    }
    
    $expected = (Get-Content $expectedFile -Raw).TrimEnd()
    
    try {
        $actual = (& $jsEngine $jsFile 2>&1) | Out-String
        $actual = $actual.TrimEnd()
        
        if($actual -eq $expected) {
            Write-Host "PASS $testName" -ForegroundColor Green
            $script:passed++
        } else {
            Write-Host "FAIL $testName" -ForegroundColor Red
            Write-Host "  Expected: $expected"
            Write-Host "  Actual:   $actual"
            $script:failed++
            $script:errors += $testName
        }
    } catch {
        Write-Host "ERROR $testName : $_" -ForegroundColor Red
        $script:failed++
        $script:errors += $testName
    }
}

Write-Host ""
Write-Host "Results: $passed passed, $failed failed, out of $($passed + $failed) total"
if($errors.Count -gt 0) {
    Write-Host "Failed tests:"
    $errors | ForEach-Object { Write-Host "  - $_" }
}
