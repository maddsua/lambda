import { createApp } from 'vue';
import { createRouter, createWebHistory } from 'vue-router';
import App from './App.vue';
import HomeView from './views/features.vue';

import './assets/main.css';

const app = createApp(App);

app.use(createRouter({
	history: createWebHistory(import.meta.env.BASE_URL),
	routes: [
		{
			path: '/',
			name: 'home',
			component: HomeView
		},
		{
			path: '/db',
			name: 'Database',
			component: () => import('./views/database.vue')
		},
		{
			path: '/vfs',
			name: 'VFS',
			component: () => import('./views/virtualfilesystem.vue')
		},
		{
			path: '/fetch',
			name: 'Fetch',
			component: () => import('./views/fetch.vue')
		}
	]
}));

app.mount('#app');
