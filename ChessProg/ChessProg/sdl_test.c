//#include <stdio.h>
//#include <SDL.h>
//#include <SDL_video.h>
//
//#define WIN_W 640
//#define WIN_H 480
//#define IMG_W 240
//#define IMG_H 296
//
//int main(void) {
//	SDL_Event e;
//	SDL_Rect rect = {10, 10, 50, 50};
//	SDL_Rect imgrect = {0, 0, IMG_W, IMG_H};
//	SDL_Surface *img = SDL_LoadBMP("test.bmp");
//	SDL_Surface *w  = SDL_SetVideoMode(WIN_W, WIN_H, 0, SDL_HWSURFACE | SDL_DOUBLEBUF);
//	int quit = 0;
//	
//	/* Initialize SDL and make sure it quits*/
//	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//		printf("ERROR: unable to init SDL: %s\n", SDL_GetError());
//		return 1;
//	}
//	atexit(SDL_Quit);
//
//	/* Create window surface*/
//
//	if (w == NULL) {
//		printf("ERROR: failed to set video mode: %s\n", SDL_GetError());
//		return 1;
//	}
//
//	/* Define the rects we need*/
//	
//
//	/* Load test spritesheet image*/
//	
//	if (img == NULL) {
//		printf("ERROR: failed to load image: %s\n", SDL_GetError());
//		return 1;
//	}
//
//	/* Set colorkey to BLUE*/
//	if (SDL_SetColorKey(img, SDL_SRCCOLORKEY, SDL_MapRGB(img->format, 0, 0, 255)) != 0) {
//		printf("ERROR: failed to set color key: %s\n", SDL_GetError());
//		SDL_FreeSurface(img);
//		return 1;
//	}
//
//
//	while (!quit) {
//		/* Clear window to BLACK*/
//		if (SDL_FillRect(w,0,0) != 0) {
//			printf("ERROR: failed to draw rect: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Green rectangle button*/
//		if (SDL_FillRect(w, &rect, SDL_MapRGB(w->format, 0, 255, 0)) != 0) {
//			printf("ERROR: failed to draw rect: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Draw image sprite*/
//		if (SDL_BlitSurface(img, &imgrect, w, 0) != 0) {
//			SDL_FreeSurface(img);
//			printf("ERROR: failed to blit image: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Advance to next sprite*/
//		imgrect.x += imgrect.w;
//		if (imgrect.x >= img->w) {
//			imgrect.x = 0;
//			imgrect.y += imgrect.h;
//			if (imgrect.y >= img->h) imgrect.y = 0;
//		}
//
//		/* We finished drawing*/
//		if (SDL_Flip(w) != 0) {
//			printf("ERROR: failed to flip buffer: %s\n", SDL_GetError());
//			break;
//		}
//
//		/* Poll for keyboard & mouse events*/
//
//		while (SDL_PollEvent(&e) != 0) {
//			switch (e.type) {
//				case (SDL_QUIT):
//					quit = 1;
//					break;
//				case (SDL_KEYUP):
//					if (e.key.keysym.sym == SDLK_ESCAPE) quit = 1;
//					break;
//				case (SDL_MOUSEBUTTONUP):
//					if ((e.button.x > rect.x) && (e.button.x < rect.x + rect.w) && (e.button.y > rect.y) && (e.button.y < rect.y+rect.h))
//						quit = 1;
//					break;
//				default:
//					break;
//			}
//		}
//
//		/* Wait a little before redrawing*/
//		SDL_Delay(1000);
//	}
//
//	SDL_FreeSurface(img);
//	return 0;
//}