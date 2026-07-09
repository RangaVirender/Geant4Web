// Minimal canvas spectrum plot (lin/log y), DPI-aware, resizes with its pane.
export class SpectrumPlot {
  private ctx: CanvasRenderingContext2D;
  private data: Float64Array | null = null;
  private binWidth = 1;
  logY = false;

  constructor(private canvas: HTMLCanvasElement) {
    this.ctx = canvas.getContext('2d')!;
    new ResizeObserver(() => this.draw()).observe(canvas.parentElement!);
  }

  set(data: Float64Array, binWidth: number) {
    this.data = data;
    this.binWidth = binWidth;
    this.draw();
  }

  draw() {
    const pane = this.canvas.parentElement!;
    const dpr = devicePixelRatio || 1;
    const w = pane.clientWidth, h = pane.clientHeight;
    // CSS (absolute inset:0) controls display size; only set the buffer size.
    this.canvas.width = w * dpr;
    this.canvas.height = h * dpr;
    const ctx = this.ctx;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, w, h);
    if (!this.data) return;

    const mL = 56, mR = 14, mT = 30, mB = 34;
    const pw = w - mL - mR, ph = h - mT - mB;
    const d = this.data;
    const n = d.length;
    let yMax = 0;
    for (let i = 0; i < n; i++) if (d[i] > yMax) yMax = d[i];
    if (yMax === 0) yMax = 1;
    const yMinLog = 0.5;
    const toY = (v: number) => {
      if (this.logY) {
        const lv = Math.log10(Math.max(v, yMinLog));
        const l0 = Math.log10(yMinLog), l1 = Math.log10(yMax * 1.3);
        return mT + ph - ((lv - l0) / (l1 - l0)) * ph;
      }
      return mT + ph - (v / (yMax * 1.08)) * ph;
    };

    // grid + axes
    ctx.strokeStyle = '#2c3a52';
    ctx.fillStyle = '#8fa1bd';
    ctx.font = '11px system-ui';
    ctx.lineWidth = 1;
    const eMax = n * this.binWidth;
    const xTicks = 8;
    for (let i = 0; i <= xTicks; i++) {
      const x = mL + (pw * i) / xTicks;
      ctx.beginPath(); ctx.moveTo(x, mT); ctx.lineTo(x, mT + ph); ctx.stroke();
      ctx.fillText(`${Math.round((eMax * i) / xTicks)}`, x - 12, h - 14);
    }
    ctx.fillText('energy (keV)', mL + pw / 2 - 30, h - 2);
    const yTicks = 5;
    for (let i = 0; i <= yTicks; i++) {
      const y = mT + (ph * i) / yTicks;
      ctx.beginPath(); ctx.moveTo(mL, y); ctx.lineTo(mL + pw, y); ctx.stroke();
      let v: number;
      if (this.logY) {
        const l0 = Math.log10(yMinLog), l1 = Math.log10(yMax * 1.3);
        v = Math.pow(10, l1 - ((l1 - l0) * i) / yTicks);
      } else {
        v = yMax * 1.08 * (1 - i / yTicks);
      }
      ctx.fillText(v >= 1000 ? v.toExponential(1) : v.toFixed(v < 10 ? 1 : 0), 4, y + 4);
    }

    // histogram line
    ctx.strokeStyle = '#4fa3ff';
    ctx.lineWidth = 1.4;
    ctx.beginPath();
    for (let i = 0; i < n; i++) {
      const x = mL + (pw * i) / n;
      const y = toY(d[i]);
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    }
    ctx.stroke();
  }
}
