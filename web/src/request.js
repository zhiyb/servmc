import http from 'http';

let request = (options) => {
	return new Promise((resolve, reject) => {
		// let token = ;
		let hostname = options.hostname;
		let port = options.port;
		let path = options.path;
		let method = options.method ? options.method : "GET";
		let query = options.query;
		let data = options.data;

		if (query) {	//get查询用的参数
			let urlQuery = '';
			for (let k in query) {
				urlQuery += `&${k}=${query[k]}`;
			}
			// path = path + '?token=' + token + urlQuery;
			path = path + urlQuery;
		}

		let req = null;
		if (method === 'GET') {
			req = http.get(
				{
					hostname: hostname,
					path: path,
					port: port,
					method: 'GET',
				},
				resolve
			);
		}
		else if (method === 'POST') {
			req = http.request(
				{
					hostname: hostname,
					path: path,
					port: port,
					method: 'POST',
					headers: {
						'Content-Type':
							Buffer.isBuffer(data) ?
								'application/octet-stream' :
								(typeof data === 'object') ?
									'application/json' :
									'text/plain',
					},
				},
				resolve
			);
			// console.log(Buffer.isBuffer(data));
			req.write(
				Buffer.isBuffer(data) ?
					data :
					(typeof data === 'object') ?
						JSON.stringify(data) :
						data
			);
		}

		req.end();
	})
		.then(response => {
			return new Promise((resolve, reject) => {
				switch (response.statusCode) {
					case 200:
						response.setEncoding('utf8');
						let body = '';
						response.on('data', (chunk) => {
							body += chunk;
						});
						response.on('end', () => {
							if (response.headers['content-type'].indexOf("application/json") !== -1)
								resolve(JSON.parse(body));
							else
								resolve(body);
						});
						break;
					case 404:
						resolve({
							retCode: 1,
							retMsg: '服务器故障',
						});
						break;
					default:
						resolve({
							retCode: 1,
							retMsg: '您的网络异常',
						});
						break;
				}
			})
		})
		.then((ret) => {	//错误处理，这个过程会尽可能处理通用错误，菲通用的错误再抛出做具体特定处理
			if (ret.retCode)	//如果有错误代码则按照处理
				switch (ret.retCode) {
					case 1:		//网络异常
						return Promise.reject(ret);		//抛出异常
					case 100:	//未登录
						return Promise.reject(ret);		//抛出异常
					default:	//其他错误
						return Promise.reject(ret);		//抛出异常
				}
			else	//retCode为0或没有为成功
				return Promise.resolve(ret);	//成功，下一个then
		});
};

export default request;
