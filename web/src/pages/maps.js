import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'

import './maps.css';

class maps extends Component {
	canvas = null;
	canvasContext = null;
	images = {};

	test = () => {
		if (this.canvas) {
			for (let x = 0; x < 100; x++)for (let y = 0; y < 100; y++) {
				switch (Math.round(Math.random() * 10)) {
					case 1:
						this.canvasContext.drawImage(this.images.diamond_ore, x * 16, y * 16);
						break;
					case 2:
						this.canvasContext.drawImage(this.images.emerald_ore, x * 16, y * 16);
						break;
					default:
						this.canvasContext.drawImage(this.images.stone, x * 16, y * 16);
				}
			}
		}
	}

	componentDidMount() {
		(this.images.diamond_ore = new Image()).src = diamond_ore;
		(this.images.emerald_ore = new Image()).src = emerald_ore;
		(this.images.stone = new Image()).src = stone;
	}

	render() {
		return (
			<div>
				<button onClick={this.test}>test</button>
				<br />
				<canvas
					className="canvas"
					ref={canvas => { this.canvas = canvas; this.canvasContext = canvas.getContext("2d"); }}
					height="1600px"
					width="1600px"
				></canvas>
			</div >
		);
	}
}

export default maps;