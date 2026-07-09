// three.js view: detector geometry + sampled particle tracks.
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import type { SimConfig, TracksPayload } from './types';

const PROC_COLORS: Record<string, number> = {
  compt: 0xffb454, phot: 0xff5470, conv: 0xc678dd, Rayl: 0x56b6c2,
};
const GAMMA_COLOR = 0x4fa3ff;
const ELECTRON_COLOR = 0x98c379;

export class View3D {
  private scene = new THREE.Scene();
  private camera: THREE.PerspectiveCamera;
  private renderer: THREE.WebGLRenderer;
  private controls: OrbitControls;
  private detectorGroup = new THREE.Group();
  private trackGroup = new THREE.Group();

  constructor(canvas: HTMLCanvasElement) {
    this.renderer = new THREE.WebGLRenderer({ canvas, antialias: true, alpha: true });
    this.camera = new THREE.PerspectiveCamera(45, 1, 1, 5000);
    this.camera.position.set(180, 120, -220);
    this.controls = new OrbitControls(this.camera, canvas);
    this.controls.enableDamping = true;
    this.scene.add(new THREE.AmbientLight(0xffffff, 0.7));
    const dir = new THREE.DirectionalLight(0xffffff, 1.2);
    dir.position.set(1, 2, -1);
    this.scene.add(dir, this.detectorGroup, this.trackGroup);
    this.scene.add(new THREE.AxesHelper(40));

    const pane = canvas.parentElement!;
    new ResizeObserver(() => this.resize()).observe(pane);
    this.resize();
    this.animate();
  }

  private resize() {
    const pane = this.renderer.domElement.parentElement!;
    const w = pane.clientWidth, h = pane.clientHeight;
    this.renderer.setSize(w, h, false);
    this.renderer.setPixelRatio(devicePixelRatio || 1);
    this.camera.aspect = w / Math.max(1, h);
    this.camera.updateProjectionMatrix();
  }

  private animate = () => {
    requestAnimationFrame(this.animate);
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  };

  // Geant4 z-axis (beam axis) is mapped to three.js -z toward viewer default.
  setGeometry(cfg: SimConfig) {
    this.detectorGroup.clear();
    const crystalMat = new THREE.MeshPhysicalMaterial({
      color: 0x7fb7ff, transparent: true, opacity: 0.32, roughness: 0.25,
      side: THREE.DoubleSide,
    });
    const housingMat = new THREE.MeshStandardMaterial({
      color: 0x9aa4b2, transparent: true, opacity: 0.15, side: THREE.DoubleSide,
    });
    let crystal: THREE.Mesh;
    const rotX = Math.PI / 2; // cylinders: three.js y-axis -> Geant4 z-axis
    if (cfg.shape === 'box') {
      crystal = new THREE.Mesh(new THREE.BoxGeometry(cfg.boxX, cfg.boxY, cfg.height), crystalMat);
    } else {
      const r = cfg.diameter / 2;
      crystal = new THREE.Mesh(new THREE.CylinderGeometry(r, r, cfg.height, 48), crystalMat);
      crystal.rotation.x = rotX;
      if (cfg.shape === 'well') {
        const bore = new THREE.Mesh(
          new THREE.CylinderGeometry(cfg.wellDiameter / 2, cfg.wellDiameter / 2, cfg.wellDepth, 32),
          new THREE.MeshStandardMaterial({ color: 0x10141a }));
        bore.rotation.x = rotX;
        bore.position.z = -cfg.height / 2 + cfg.wellDepth / 2;
        this.detectorGroup.add(bore);
      }
    }
    this.detectorGroup.add(crystal);
    if (cfg.housingThickness > 0) {
      let housing: THREE.Mesh;
      const t = cfg.housingThickness;
      if (cfg.shape === 'box') {
        housing = new THREE.Mesh(
          new THREE.BoxGeometry(cfg.boxX + 2 * t, cfg.boxY + 2 * t, cfg.height + 2 * t), housingMat);
      } else {
        const r = cfg.diameter / 2 + t;
        housing = new THREE.Mesh(new THREE.CylinderGeometry(r, r, cfg.height + 2 * t, 48), housingMat);
        housing.rotation.x = rotX;
      }
      this.detectorGroup.add(housing);
    }
    // Source marker
    const srcZ = -cfg.height / 2 - cfg.housingThickness -
      (cfg.sourceType === 'point' ? cfg.sourceDistance : 10);
    const src = new THREE.Mesh(new THREE.SphereGeometry(3, 16, 16),
      new THREE.MeshBasicMaterial({ color: 0xff5470 }));
    src.position.z = srcZ;
    this.detectorGroup.add(src);
    // frame camera
    const span = Math.max(cfg.diameter, cfg.height, Math.abs(srcZ)) * 1.6;
    this.camera.position.set(span, span * 0.6, srcZ * 0.4);
    this.controls.target.set(0, 0, 0);
  }

  setTracks(payload: TracksPayload) {
    this.trackGroup.clear();
    const byColor = new Map<number, number[]>();
    for (const t of payload.tracks) {
      let color: number;
      if (t.pdg === 22) {
        const key = Object.keys(PROC_COLORS).find((k) => t.proc.startsWith(k));
        color = key ? PROC_COLORS[key] : GAMMA_COLOR;
        color = GAMMA_COLOR; // line segment colored as gamma; vertex markers show process
        if (key) this.addInteractionMarker(t.p1, PROC_COLORS[key]);
      } else {
        color = ELECTRON_COLOR;
      }
      const arr = byColor.get(color) ?? [];
      arr.push(t.p0[0], t.p0[1], t.p0[2], t.p1[0], t.p1[1], t.p1[2]);
      byColor.set(color, arr);
    }
    for (const [color, verts] of byColor) {
      const geo = new THREE.BufferGeometry();
      geo.setAttribute('position', new THREE.Float32BufferAttribute(verts, 3));
      this.trackGroup.add(new THREE.LineSegments(geo,
        new THREE.LineBasicMaterial({ color, transparent: true, opacity: 0.65 })));
    }
  }

  private addInteractionMarker(p: [number, number, number], color: number) {
    const m = new THREE.Mesh(new THREE.SphereGeometry(0.9, 8, 8),
      new THREE.MeshBasicMaterial({ color }));
    m.position.set(p[0], p[1], p[2]);
    this.trackGroup.add(m);
  }
}
