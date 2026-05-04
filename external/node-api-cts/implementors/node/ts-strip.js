import { register } from 'node:module';
import 'amaro/strip';

register('./ts-loader.js', import.meta.url);
