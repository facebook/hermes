$boostComponents = @(
  "algorithm",
  "asio",
  "assert", 
  "bind",
  "concept_check",
  "config",
  "container_hash",
  "core",
  "detail",
  "function",
  "integer",
  "iterator", 
  "move",
  "mpl",
  "preprocessor",
  "range",
  "static_assert",
  "throw_exception",
  "type_index",
  "type_traits",
  "utility",
  "variant"
)

function Clean-BoostComponent {
    param($componentPath)
    
    Write-Host "  Cleaning up $componentPath..." -ForegroundColor Yellow
    
    # Remove .git directory to eliminate Git repository
    if (Test-Path "$componentPath\.git") {
        Remove-Item "$componentPath\.git" -Recurse -Force
    }
    
    # Get all items in the component directory
    $items = Get-ChildItem $componentPath
    
    foreach ($item in $items) {
        # Keep these essential folders
        if ($item.Name -in @("meta", "include")) {
            Write-Host "    Keeping folder: $($item.Name)" -ForegroundColor Green
            continue
        }
        
        # Keep these essential files
        if ($item.Name -match "\.(cpp|hpp|h)$" -or $item.Name -eq "CMakeLists.txt") {
            Write-Host "    Keeping file: $($item.Name)" -ForegroundColor Green
            continue
        }
        
        # Remove everything else
        Write-Host "    Removing: $($item.Name)" -ForegroundColor Red
        Remove-Item $item.FullName -Recurse -Force
    }
}

foreach ($component in $boostComponents) {
    Write-Host "Downloading boost $component..." -ForegroundColor Cyan
    
    # Clone the repository
    & git clone "https://github.com/boostorg/$component.git" --branch boost-1.76.0 --depth 1 $component
    
    if ($LASTEXITCODE -eq 0) {
        # Clean up the component after successful download
        Clean-BoostComponent $component
        Write-Host "  Successfully downloaded and cleaned $component" -ForegroundColor Green
    } else {
        Write-Host "  Failed to download $component" -ForegroundColor Red
    }
}

Write-Host "`nBoost component download and cleanup complete!" -ForegroundColor Green
Write-Host "All Git repositories have been removed and only essential files remain." -ForegroundColor Yellow
