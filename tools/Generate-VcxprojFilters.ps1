param(
  [Parameter(Mandatory=$true)] [string]$ProjectPath,
  [string]$FiltersPath = $null
)

if (-not (Test-Path $ProjectPath)) { throw "Project not found: $ProjectPath" }
if (-not $FiltersPath) { $FiltersPath = "$ProjectPath.filters" }

[xml]$proj = Get-Content -LiteralPath $ProjectPath
$ns = "http://schemas.microsoft.com/developer/msbuild/2003"

# Collect items we care about
$itemNames = @('ClCompile','ClInclude','None','ResourceCompile','CustomBuild')
$items = @()
foreach ($name in $itemNames) {
  $nodes = $proj.Project.ItemGroup.$name
  if ($nodes) { $items += $nodes }
}

# Deduplicate by (ItemName, Include)
$seen = @{}
$uni = @()
foreach ($it in $items) {
  $key = "$($it.Name)|$($it.Include)"
  if (-not $seen.ContainsKey($key)) { $seen[$key] = $true; $uni += $it }
}

# Helper: choose a filter name for a file
function Get-RootCategory($ext) {
  switch ($ext.ToLowerInvariant()) {
    '.c' { 'Source Files'; break }
    '.cc' { 'Source Files'; break }
    '.cpp' { 'Source Files'; break }
    '.cxx' { 'Source Files'; break }
    '.h' { 'Header Files'; break }
    '.hh' { 'Header Files'; break }
    '.hpp' { 'Header Files'; break }
    '.inl' { 'Header Files'; break }
    '.rc' { 'Resource Files'; break }
    '.ico' { 'Resource Files'; break }
    '.cur' { 'Resource Files'; break }
    '.bmp' { 'Resource Files'; break }
    '.png' { 'Resource Files'; break }
    '.jpg' { 'Resource Files'; break }
    '.jpeg' { 'Resource Files'; break }
    '.tga' { 'Resource Files'; break }
    '.tiff' { 'Resource Files'; break }
    '.vert' { 'Shaders'; break }
    '.frag' { 'Shaders'; break }
    '.glsl' { 'Shaders'; break }
    '.shader' { 'Shaders'; break }
    '.vertexshader' { 'Shaders'; break }
    '.fragmentshader' { 'Shaders'; break }
    default { '' }
  }
}

function Get-FilterForInclude($include) {
  $ext = [System.IO.Path]::GetExtension($include)
  $cat = Get-RootCategory $ext
  $dir = [System.IO.Path]::GetDirectoryName($include) -replace '/','\'
  if ([string]::IsNullOrWhiteSpace($cat)) {
    if ([string]::IsNullOrWhiteSpace($dir)) { return '' } else { return $dir }
  } else {
    if ([string]::IsNullOrWhiteSpace($dir)) { return $cat } else { return "$cat\$dir" }
  }
}

# Build set of filters to create
$filtersNeeded = New-Object 'System.Collections.Generic.HashSet[string]'
$mapItemToFilter = @{}
foreach ($it in $uni) {
  $inc = $it.Include
  if (-not $inc) { continue }
  $filter = Get-FilterForInclude $inc
  $mapItemToFilter[$it] = $filter
  if (-not [string]::IsNullOrWhiteSpace($filter)) {
    # Add every parent filter too (A\B\C => A, A\B, A\B\C)
    $parts = $filter -split '\\'
    $acc = @()
    foreach ($p in $parts) {
      $acc += $p
      [void]$filtersNeeded.Add(($acc -join '\'))
    }
  }
}

# Create filters XML
$filtersXml = New-Object System.Xml.XmlDocument
$decl = $filtersXml.CreateXmlDeclaration("1.0","utf-8",$null)
$filtersXml.AppendChild($decl) | Out-Null
$projNode = $filtersXml.CreateElement("Project")
$projNode.SetAttribute("ToolsVersion","4.0")
$projNode.SetAttribute("xmlns",$ns)
$filtersXml.AppendChild($projNode) | Out-Null

# ItemGroup for <Filter> entries
$igFilters = $filtersXml.CreateElement("ItemGroup",$ns)
$projNode.AppendChild($igFilters) | Out-Null

# Deterministic GUID from filter name (MD5 -> GUID format)
function New-DeterministicGuid([string]$name) {
  $md5 = [System.Security.Cryptography.MD5]::Create()
  $bytes = [System.Text.Encoding]::UTF8.GetBytes($name)
  $hash = $md5.ComputeHash($bytes)
  # Build GUID from first 16 bytes
  return New-Object Guid (,$hash)
}

foreach ($f in ($filtersNeeded | Sort-Object)) {
  $fNode = $filtersXml.CreateElement("Filter",$ns)
  $fNode.SetAttribute("Include",$f)
  $uidNode = $filtersXml.CreateElement("UniqueIdentifier",$ns)
  $uidNode.InnerText = ("{0}" -f (New-DeterministicGuid $f).ToString("B").ToUpperInvariant())
  $fNode.AppendChild($uidNode) | Out-Null
  $igFilters.AppendChild($fNode) | Out-Null
}

# ItemGroup for file mappings
$igItems = $filtersXml.CreateElement("ItemGroup",$ns)
$projNode.AppendChild($igItems) | Out-Null

foreach ($it in $uni) {
  $filter = $mapItemToFilter[$it]
  if ([string]::IsNullOrWhiteSpace($filter)) { continue }
  $node = $filtersXml.CreateElement($it.Name,$ns)
  $node.SetAttribute("Include",$it.Include)
  $filterNode = $filtersXml.CreateElement("Filter",$ns)
  $filterNode.InnerText = $filter
  $node.AppendChild($filterNode) | Out-Null
  $igItems.AppendChild($node) | Out-Null
}

$dirOut = [System.IO.Path]::GetDirectoryName($FiltersPath)
if ($dirOut -and -not (Test-Path $dirOut)) { New-Item -ItemType Directory -Path $dirOut | Out-Null }
$filtersXml.Save($FiltersPath)
Write-Host "Generated: $FiltersPath"