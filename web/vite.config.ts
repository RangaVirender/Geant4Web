import { defineConfig } from 'vite';

export default defineConfig({
  // Relative base so the site works at https://<user>.github.io/<repo>/
  base: './',
  worker: { format: 'es' },
  build: { target: 'es2022' },
});
