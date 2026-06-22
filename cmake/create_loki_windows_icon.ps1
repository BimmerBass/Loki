param(
  [Parameter(Mandatory = $true)]
  [string]$InputPng,

  [Parameter(Mandatory = $true)]
  [string]$OutputIco
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

function New-ScaledBitmap {
  param(
    [System.Drawing.Image]$Source,
    [int]$Size
  )

  $target = New-Object System.Drawing.Bitmap $Size, $Size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
  $graphics = [System.Drawing.Graphics]::FromImage($target)
  try {
    $graphics.Clear([System.Drawing.Color]::Transparent)
    $graphics.CompositingMode = [System.Drawing.Drawing2D.CompositingMode]::SourceCopy
    $graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality

    $scale = [Math]::Min($Size / [double]$Source.Width, $Size / [double]$Source.Height)
    $drawWidth = [Math]::Round($Source.Width * $scale)
    $drawHeight = [Math]::Round($Source.Height * $scale)
    $x = [Math]::Floor(($Size - $drawWidth) / 2)
    $y = [Math]::Floor(($Size - $drawHeight) / 2)

    $graphics.DrawImage($Source, $x, $y, $drawWidth, $drawHeight)
  } finally {
    $graphics.Dispose()
  }

  return $target
}

$sourceImage = [System.Drawing.Image]::FromFile($InputPng)
try {
  $scaled = New-ScaledBitmap -Source $sourceImage -Size 256
  try {
    $hIcon = $scaled.GetHicon()
    try {
      $icon = [System.Drawing.Icon]::FromHandle($hIcon)
      try {
        $stream = [System.IO.File]::Create($OutputIco)
        try {
          $icon.Save($stream)
        } finally {
          $stream.Dispose()
        }
      } finally {
        $icon.Dispose()
      }
    } finally {
    }
  } finally {
    $scaled.Dispose()
  }
} finally {
  $sourceImage.Dispose()
}
