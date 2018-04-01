import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'
import cobblestone from './maps/cobblestone.png'
import dirt from './maps/dirt.png'
import cobblestone_mossy from './maps/cobblestone_mossy.png'
import grass_path_top from './maps/grass_path_top.png'
import c1_0 from './maps/chunk-test/c1_0.png'
import c1_1 from './maps/chunk-test/c1_1.png'
import c2_0 from './maps/chunk-test/c2_0.png'

import './maps.css';

class maps extends Component {
	state = {
		mapSize: [1000, 1000],
		mapMove: [0, 0],
		mapZoom: 1,
	};

	canvas = null;
	canvasContext = null;
	images = {};

	mapScale = 1;
	mapPosition = [0, 0];

	mapMouseDownPos = null;
	mapWheelTimer = null;
	mapTouchStartPos = null;
	mapTouchStartLength = null;

	getBlock = (x, y) => {
		if (Math.abs(Math.floor(x / 16) % 2) - Math.abs(Math.floor(y / 16) % 2) === 0)
			if (Math.abs(x % 2) - Math.abs(y % 2) === 0)
				return this.images.cobblestone;
			else
				return this.images.dirt;
		else
			if (Math.abs(x % 2) - Math.abs(y % 2) === 0)
				return this.images.cobblestone_mossy;
			else
				return this.images.grass_path_top;

	}

	getChunk = (x, y, s) => {
		if (s === 1) {
			if (Math.abs(x % 2) - Math.abs(y % 2) === 0)
				return this.images.c1_0;
			else
				return this.images.c1_1;
		}
		else {
			return this.images.c2_0;
		}
	}

	draw = () => {
		if (this.canvas) {
			//处理鼠标移动
			this.mapPosition[0] += this.state.mapMove[0] / 16 / this.mapScale;
			this.mapPosition[1] += this.state.mapMove[1] / 16 / this.mapScale;
			this.mapMouseDownPos = null;
			this.setState({ mapMove: [0, 0] });
			//处理滚轮缩放
			this.mapScale *= this.state.mapZoom;
			this.setState({ mapZoom: 1 });
			this.mapWheelTimer = null;

			this.canvasContext.clearRect(0, 0, this.canvas.width, this.canvas.height)
			if (this.mapScale > 1/2) {
				for (
					let x = Math.floor(- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]);
					x <= Math.floor(this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]);
					x++
				)
					for (
						let y = Math.floor(- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]);
						y <= Math.floor(this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]);
						y++
					) {
						this.canvasContext.drawImage(
							this.getBlock(x, y),
							0, 0, 16, 16,
							this.canvas.width / 2 + (x + this.mapPosition[0]) * 16 * this.mapScale,
							this.canvas.height / 2 + (y + this.mapPosition[1]) * 16 * this.mapScale,
							16 * this.mapScale,
							16 * this.mapScale);
					}
			}
			else if (this.mapScale > 1/16/2) {
				for (
					let x = Math.floor((- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0])/16);
					x <= Math.floor((this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0])/16);
					x++
				)
					for (
						let y = Math.floor((- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1])/16);
						y <= Math.floor((this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1])/16);
						y++
					) {
						this.canvasContext.drawImage(
							this.getChunk(x, y, 1),
							0, 0, 16, 16,
							this.canvas.width / 2 + (x*16 + this.mapPosition[0]) * 16 * this.mapScale,
							this.canvas.height / 2 + (y*16 + this.mapPosition[1]) * 16 * this.mapScale,
							16 * this.mapScale *16,
							16 * this.mapScale *16);
					}
			}
			else  if (this.mapScale > 1/16/16/2) {
				for (
					let x = Math.floor((- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0])/16/16);
					x <= Math.floor((this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0])/16/16);
					x++
				)
					for (
						let y = Math.floor((- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1])/16/16);
						y <= Math.floor((this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1])/16/16);
						y++
					) {
						this.canvasContext.drawImage(
							this.getChunk(x, y, 2),
							0, 0, 16, 16,
							this.canvas.width / 2 + (x*16*16 + this.mapPosition[0]) * 16 * this.mapScale,
							this.canvas.height / 2 + (y*16*16 + this.mapPosition[1]) * 16 * this.mapScale,
							16 * this.mapScale *16*16,
							16 * this.mapScale *16*16);
					}
			}
		}
	}

	componentDidMount() {
		(this.images.diamond_ore = new Image()).src = diamond_ore;
		(this.images.emerald_ore = new Image()).src = emerald_ore;
		(this.images.stone = new Image()).src = stone;
		(this.images.cobblestone = new Image()).src = cobblestone;
		(this.images.dirt = new Image()).src = dirt;
		(this.images.cobblestone_mossy = new Image()).src = cobblestone_mossy;
		(this.images.grass_path_top = new Image()).src = grass_path_top;
		(this.images.c1_0 = new Image()).src = c1_0;
		(this.images.c1_1 = new Image()).src = c1_1;
		(this.images.c2_0 = new Image()).src = c2_0;
	}

	handleMouseDown = event => {
		event.preventDefault();
		this.mapMouseDownPos = [event.clientX, event.clientY];
	}
	handleMouseMove = event => {
		if (this.mapMouseDownPos) {
			event.preventDefault();
			this.setState({
				mapMove: [
					event.clientX - this.mapMouseDownPos[0],
					event.clientY - this.mapMouseDownPos[1]
				]
			});
		}
	}
	handleMouseUp = () => {
		if (this.mapMouseDownPos) {
			this.draw();
		}
	}

	handleWheel = event => {
		if (this.mapWheelTimer) clearTimeout(this.mapWheelTimer);	//如果已经有定时器清除掉
		this.mapWheelTimer = setTimeout(this.draw, 200);	//滚动停止后0.2s才重新渲染

		let mapScaleNew = this.mapScale * this.state.mapZoom * (1 + event.deltaY / 500);
		if (mapScaleNew < 100 && mapScaleNew > 1/16/16) {
			this.setState({ mapZoom: this.state.mapZoom * (1 + event.deltaY / 500) });
		}
	}

	handleTouchStart = event => {
		if (event.targetTouches.length === 1) {
			event.preventDefault();
			this.mapTouchStartPos = [event.targetTouches[0].pageX, event.targetTouches[0].pageY];
		}
		else if (event.targetTouches.length === 2) {
			event.preventDefault();
			this.mapTouchStartLength = Math.sqrt(Math.pow(event.targetTouches[0].pageX - event.targetTouches[1].pageX, 2) + Math.pow(event.targetTouches[0].pageY - event.targetTouches[1].pageY, 2));
		}
	}
	handleTouchMove = event => {
		if (event.targetTouches.length === 1) {
			event.preventDefault();
			if (this.mapTouchStartPos) {
				this.setState({
					mapMove: [
						event.targetTouches[0].pageX - this.mapTouchStartPos[0],
						event.targetTouches[0].pageY - this.mapTouchStartPos[1]
					]
				});
			}
		}
		else if (event.targetTouches.length === 2) {
			event.preventDefault();
			this.setState({
				mapZoom: Math.sqrt(Math.pow(event.targetTouches[0].pageX - event.targetTouches[1].pageX, 2) + Math.pow(event.targetTouches[0].pageY - event.targetTouches[1].pageY, 2)) / this.mapTouchStartLength
			});
		}
	}
	handleTouchEnd = () => {
		if (this.mapTouchStartPos) {
			this.draw();
		}
	}

	componentWillUnmount() {
		if(this.map){
			this.map.removeEventListener("touchstart", this.handleTouchStart);
			this.map.removeEventListener("touchmove", this.handleTouchStart);
			this.map.removeEventListener("touchend", this.handleTouchStart);
		}
	}

	render() {
		return (
			<div>
				<button onClick={this.draw}>draw</button>
				<br />
				<div className="map" style={{ height: this.state.mapSize[0], width: this.state.mapSize[1] }}
					onMouseDown={this.handleMouseDown}
					onMouseMove={this.handleMouseMove}
					onMouseUp={this.handleMouseUp}
					onMouseOut={this.handleMouseUp}
					onWheel={this.handleWheel}	//滚轮
					// onTouchStart={this.handleTouchStart}
					// onTouchMove={this.handleTouchMove}
					// onTouchEnd={this.handleTouchEnd}
					ref={map => {
						if(this.map){
							this.map.removeEventListener("touchstart", this.handleTouchStart);
							this.map.removeEventListener("touchmove", this.handleTouchStart);
							this.map.removeEventListener("touchend", this.handleTouchStart);
						}
						if (map) {
							map.addEventListener("touchstart", this.handleTouchStart, false);	//onTouchStart会导致preventDefault报错 只能这样
							map.addEventListener("touchmove", this.handleTouchMove, false);
							map.addEventListener("touchend", this.handleTouchEnd, false);
						}
						this.map = map;
					}}
				>
					<canvas
						className="canvas"
						ref={canvas => {
							this.canvas = canvas;
							if (canvas) {
								this.canvasContext = canvas.getContext("2d");
								this.canvasContext.imageSmoothingEnabled = false;
							}
						}}
						height={this.state.mapSize[0] * 3}
						width={this.state.mapSize[1] * 3}
						style={{
							left: this.state.mapMove[0] - this.state.mapSize[0] + (this.state.mapSize[0] * 3 - this.state.mapSize[0] * 3 * this.state.mapZoom) / 2,
							top: this.state.mapMove[1] - this.state.mapSize[1] + (this.state.mapSize[1] * 3 - this.state.mapSize[1] * 3 * this.state.mapZoom) / 2,
							width: this.state.mapSize[0] * 3 * this.state.mapZoom,
							height: this.state.mapSize[1] * 3 * this.state.mapZoom,
						}}
					></canvas>
				</div>
			</div >
		);
	}
}

export default maps;